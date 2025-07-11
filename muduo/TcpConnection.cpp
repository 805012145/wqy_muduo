#include "TcpConnection.h"
#include "Callbacks.h"
#include "logger.h"
#include "Channel.h"
#include "Socket.h"
#include <asm-generic/socket.h>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <sys/socket.h>
#include <sys/types.h>
#include <memory>
#include <unistd.h>
namespace mymuduo {
    namespace base {
        static EventLoop* CheckLoopNotNull(EventLoop *loop) {
            if (loop == nullptr) {
                LOG_FATAL("%s:%s:%d mainLoop is nullptr", __FILE__, __FUNCTION__, __LINE__);
            }
            return loop;
        }
        TcpConnection::TcpConnection(EventLoop *loop, 
                const std::string &name, 
                int sockfd, 
                const InetAddress &localAddr, 
                const InetAddress &peerAddr)
       :loop_(CheckLoopNotNull(loop)),
        name_(name),
        state_(kConnecting),
        socket_(new Socket(sockfd)),
        channel_(new Channel(loop, sockfd)),
        localAddr_(localAddr),
        peerAddr_(peerAddr),
        highWaterMark_(64 * 1024 * 1024) { // 设置高水位线为64MB
            channel_->setReadCallBack(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
            channel_->setWriteCallBack(std::bind(&TcpConnection::handleWrite, this));
            channel_->setCloseCallBack(std::bind(&TcpConnection::handleClose, this));
            channel_->setErrorCallBack(std::bind(&TcpConnection::handleError, this));
            LOG_INFO("TcpConnection::ctor[%s] at fd=%d \n", name_.c_str(), sockfd);
            socket_->setKeepAlive(true);
        }
        TcpConnection::~TcpConnection() {
            LOG_INFO("TcpConnection::dtor[%s] at fd=%d state=%d\n", name_.c_str(), channel_->fd(), state_.load());
            assert(state_ == kDisconnected); // 确保连接已断开
        }

        void TcpConnection::send(const std::string &buf) {
            assert(state_ == kConnected); // 确保连接已建立
            if (loop_->isInLoopthread()) {
                sendInLoop(buf.data(), static_cast<int>(buf.size())); // 如果在事件循环线程中，直接发送
            } else {
                loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, buf.data(), static_cast<int>(buf.size()))); // 否则在事件循环中发送
            }
        }

        void TcpConnection::shutdown() {
            if (state_ == kConnected) {
                setState(kDisconnecting);
                loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this)); // 在事件循环中执行断开连接
            }
        }

        void TcpConnection::conneectEstableished() {
            setState(kConnected);
            channel_->tie(shared_from_this());
            channel_->enableReading(); // 启用读事件

            // 执行连接建立的回调
            connectionCallback_(shared_from_this());
            LOG_INFO("TcpConnection::conneectEstableished() - fd=%d, name=%s",
                     channel_->fd(), name_.c_str());
        }
        void TcpConnection::conneectDestroyed() {
            if (state_ == kConnected) {
                setState(kDisconnected);
                channel_->disableAll();
                connectionCallback_(shared_from_this()); // 执行连接关闭的回调
            }
            channel_->remove(); // 从事件循环中移除channel
        }

        void TcpConnection::handleRead(Timestamp receiveTime) {
            int saveErrno = 0;
            ssize_t n = inputBuffer_.readFd(channel_->fd(), &saveErrno);
            if (n > 0) {
                // 已建立的连接的用户，有可读事件发生
                messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
            } else if (n == 0) {
                // 对端关闭连接
                LOG_INFO("TcpConnection::handleRead() - connection closed by peer");
                handleClose();
            } else {
                // 发生错误
                LOG_ERROR("TcpConnection::handleRead() - read error, errno=%d", saveErrno);
                handleError();
            }
        }
        void TcpConnection::handleWrite() {
            if (channel_->isWriting()) {
                int saveErrno = 0;
                ssize_t n = outputBuffer_.writeFd(channel_->fd(), &saveErrno);
                if (n > 0) {
                    outputBuffer_.retrieve(n);
                    if (outputBuffer_.readableBytes() == 0) {
                        channel_->disableWriting(); // 如果输出缓冲区没有数据了，禁用写事件
                        if (writeCompleteCallback_) {
                            loop_->queueInLoop(
                                std::bind(writeCompleteCallback_, shared_from_this()));
                        }
                        if (state_ == kDisconnecting) {
                            shutdownInLoop(); // 如果处于断开连接状态，调用shutdownInLoop
                        }
                    }
                } else {
                    LOG_ERROR("TcpConnection::handleWrite() - channel is not writing");
                }
            } else {
                LOG_ERROR("TcpConnection fd=%d is down, no more writing", channel_->fd());
            }
        }
        void TcpConnection::handleClose() {
            LOG_INFO("TcpConnection::handleClose() - fd=%d", channel_->fd());
            assert(state_ == kConnected || state_ == kDisconnecting);
            setState(kDisconnected);
            channel_->disableAll(); // 禁用所有事件
            TcpConnectionPtr connPtr(shared_from_this());
            connectionCallback_(connPtr); // 执行连接关闭的回调
            closeCallback_(connPtr);      // 执行关闭连接的回调 执行的是TcpServer::removeConnection回调方法   // must be the last line
        }
        void TcpConnection::handleError() {
            LOG_ERROR("TcpConnection::handleError() - fd=%d", channel_->fd());
            int optval;
            socklen_t optlen = sizeof(optval);
            int err = 0;
            if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
                err = errno;
            } else {
                err = optval;
            }
            LOG_ERROR("TcpConnection::handleError() name=%s - SO_ERROR=%d", name_.c_str(), err);
        }

        void TcpConnection::sendInLoop(const void *data, int len) {
            ssize_t nwrote = 0;
            size_t remaining = static_cast<size_t>(len);
            bool faultError = false;
            if (state_ == kDisconnected) {
                LOG_ERROR("TcpConnection::sendInLoop() - connection is disconnected, fd=%d", channel_->fd());
                return;
            }
            if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
                // 如果当前对写事件不感兴趣，并且输出缓冲区没有数据
                nwrote = ::write(channel_->fd(), data, len);
                if (nwrote >= 0) {
                    remaining = len - nwrote; // 更新剩余未写入的字节数
                    if (remaining == 0 && writeCompleteCallback_) {
                        // 如果一次性写入成功，则不用给channel设置epollout事件
                        loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                    }
                } else {
                    nwrote = 0; // 写入失败，重置已写入字节数
                    if (errno != EWOULDBLOCK) {
                        LOG_ERROR("TcpConnection::sendInLoop() - write error, errno=%d", errno);
                        if (errno == EPIPE || errno == ECONNRESET) {
                            faultError = true; // 如果是管道破裂或连接重置，标记为错误
                        }
                    }
                }
            }
            // 如果没有发生错误且还有剩余数据未写入，则剩余事件保存到缓冲区
            // 注册epollout事件,poller发现tcp的发送缓冲区有空间，会通知相应的sock-channel执行相应的回调writeCallBack
            // 也就是调用TcpConnection::handleWrite()方法, 把发送缓冲区的数据全部发送完成
            if (!faultError && remaining > 0) {
                // 目前发送缓冲区的待发送数据长度
                size_t oldLen = outputBuffer_.readableBytes();
                if (oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_) {
                    loop_->queueInLoop(
                        std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
                }
                outputBuffer_.append(static_cast<const char*>(data) + nwrote, remaining);
                if (!channel_->isWriting()) {
                    channel_->enableWriting(); // 如果当前没有写事件，启用写事件
                }
            }
        }
        void TcpConnection::shutdownInLoop() {
            if (!channel_->isWriting()) {
                // 说明outputBuffer_中没有数据需要发送
                socket_->shutdownWrite(); // 关闭写端
            }
        }
    }
}