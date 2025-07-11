#include "Acceptor.h"
#include "InetAddress.h"
#include "logger.h"
#include <asm-generic/errno-base.h>
#include <cerrno>
#include <functional>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace mymuduo {
    namespace base {
        static int createNonblocking() {
            int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
            if (sockfd < 0) {
                LOG_FATAL("%s:%s:%d listen socket create err:%d \n", __FILE__, __FUNCTION__, __LINE__, errno);
            }
            return sockfd;
        }
        Acceptor::Acceptor(EventLoop *loop, const InetAddress& addr, bool reusePort)
        : loop_(loop)
        , acceptSocket_(createNonblocking())
        , acceptChannel_(loop, acceptSocket_.fd())
        , listening_(false) {
            acceptSocket_.setReusePort(reusePort);
            acceptSocket_.setReuseAddr(true);
            acceptSocket_.bindAddress(addr);
            acceptChannel_.setReadCallBack(
                std::bind(&Acceptor::handleRead, this)
            );
        }
        Acceptor::~Acceptor() {
            acceptChannel_.disableAll();
            acceptChannel_.remove();
        }

        
        void Acceptor::listen() {
            listening_ = true;
            acceptSocket_.listen();
            acceptChannel_.enableReading(); // 注册到epoll上监听
        }

        void Acceptor::handleRead() {
            InetAddress peerAddr;
            int connfd = acceptSocket_.accept(&peerAddr);
            if (connfd > 0) {
                if (newConnectionCallback_) {
                    newConnectionCallback_(connfd, peerAddr); // 轮询subLoop，唤醒，分发当前新客户端的Channel
                } else {
                    ::close(connfd);
                }
            } else {
                LOG_ERROR("%s:%s:%d accept err:%d \n", __FILE__, __FUNCTION__, __LINE__, errno);
                if (errno == EMFILE) {
                    LOG_ERROR("%s:%s:%d sockfd reach limit \n", __FILE__, __FUNCTION__, __LINE__);
                }
            }
        }
    }
}