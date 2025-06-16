#include "TcpServer.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "logger.h"
#include "EventLoopThread.h"
#include <functional>


namespace mymuduo {
    namespace base {
        EventLoop* CheckLoopNotNull(EventLoop *loop) {
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
            
        }
        void TcpServer::removeConnection(const TcpConnectionPtr &conn) {

        }
        void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {

        }
    }
}