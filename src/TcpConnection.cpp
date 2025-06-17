#include "TcpConnection.h"
#include "logger.h"
#include "Channel.h"
#include "Socket.h"
#include <cassert>
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
    }
}