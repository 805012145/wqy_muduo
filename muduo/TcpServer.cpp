#include "TcpServer.h"
#include "Acceptor.h"
#include "Callbacks.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "InetAddress.h"
#include "logger.h"
#include "EventLoopThread.h"
#include "TcpConnection.h"
#include <cstdio>
#include <functional>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>


namespace mymuduo {
    namespace base {
        static EventLoop* CheckLoopNotNull(EventLoop *loop) {
            if (loop == nullptr) {
                LOG_FATAL("%s:%s:%d mainLoop is nullptr", __FILE__, __FUNCTION__, __LINE__);
            }
            return loop;
        }
        TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &nameArg, Option option)
        : loop_(CheckLoopNotNull(loop))
        , ipPort_(listenAddr.toIpPort())
        , name_(nameArg)
        , acceptor_(new Acceptor(loop, listenAddr, option))
        , threadPool_(new EventLoopThreadPool(loop, name_))
        , connectionCallback_()
        , messageCallback_()
        , nextConnId_(1)
        , started_(0) {
            acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this,
                 std::placeholders::_1, std::placeholders::_2));
        }
        TcpServer::~TcpServer() {
            for (auto &item : connections_) {
                TcpConnectionPtr conn(item.second);
                item.second.reset(); // 释放TcpConnection对象
                conn->getLoop()->runInLoop(std::bind(&TcpConnection::conneectDestroyed, conn));
                LOG_INFO("TcpServer::~TcpServer - TcpConnection[%s] is destroyed \n", conn->name().c_str());
            }
        }


        // 开启服务器监听(开启listen)
        void TcpServer::start() {
            // 如果已经开启了监听，则不再重复开启
            if (started_++ == 0) {
                threadPool_->start(threadInitCallback_); // 启动线程池
                // 不能直接执行acceptor->listen(),因为要确保acceptor在mainloop线程中执行
                // 设计原则：遵循事件驱动框架的设计，所有 I/O 操作由 EventLoop 管理。
                loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
                LOG_INFO("TcpServer[%s] start listen on %s \n", name_.c_str(), ipPort_.c_str());
            }
        }
        
        void TcpServer::newConnection(int sockFd, const InetAddress &peerAddr) {
            EventLoop *ioLoop = threadPool_->getNextLoop();
            char buf[64];
            snprintf(buf, sizeof(buf), "-%s#%d", ipPort_.c_str(), nextConnId_);
            ++nextConnId_;
            std::string connName = name_ + buf;
            LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s \n",
                name_.c_str(),  connName.c_str(), peerAddr.toIpPort().c_str());

            sockaddr_in local;
            bzero(&local, sizeof(local));
            socklen_t addrLen = sizeof(local);
            if(::getsockname(sockFd, (sockaddr*)&local, &addrLen) < 0) {
                LOG_ERROR("TcpServer::newConnection - getsockname error");
            } 
            InetAddress localAddr(local);

            // 根据连接成功的fd, 创建TcpConnection对象
            TcpConnectionPtr conn(new TcpConnection(ioLoop, 
                                                connName, 
                                                sockFd, 
                                                localAddr,
                                                peerAddr));
            connections_[connName] = conn;
            conn->setConnectionCallback(connectionCallback_);
            conn->setMessageCallback(messageCallback_);
            conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
            conn->setWriteCompleteCallback(writeCompleteCallback_);
            ioLoop->runInLoop(std::bind(&TcpConnection::conneectEstableished, conn));
        }
        void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
            loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
        }
        void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
            LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s \n", name_.c_str(), conn->name().c_str());
            connections_.erase(conn->name());
            EventLoop *ioLoop = conn->getLoop();
            ioLoop->queueInLoop(std::bind(&TcpConnection::conneectDestroyed, conn));
        }
    }
}