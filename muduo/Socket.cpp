#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/tcp.h>

#include "Socket.h"
#include "InetAddress.h"
#include "logger.h"
namespace mymuduo {
    namespace base {
        Socket::~Socket() {
            ::close(sockFd_);
        }

        void Socket::bindAddress(const InetAddress& localAddr) {
            if (::bind(sockFd_, (struct sockaddr *)localAddr.getSockAddr(), sizeof(struct sockaddr_in)) != 0) {
                LOG_FATAL("bind sockfd:%d fail \n", sockFd_);
            }
        }
        void Socket::listen() {
            if (::listen(sockFd_, 1024) != 0) {
                LOG_FATAL("listen sockfd:%d fail \n", sockFd_);
            }
        }
        int Socket::accept(InetAddress *peerAddr) {
            sockaddr_in addr;
            socklen_t len;
            bzero(&addr, sizeof(addr));
            int connfd = ::accept(sockFd_, (struct sockaddr *) &addr, &len);
            if (connfd >= 0) {
                peerAddr->setSockAddr(addr);
            }
            return connfd;
        }

        void Socket::shutdownWrite() {
            if(::shutdown(sockFd_, SHUT_WR) < 0) {
                LOG_ERROR("shutdownWrite error");
            }
        }

        void Socket::setTcpNoDelay(bool on) {
            int optval = on ? 1 : 0;
            ::setsockopt(sockFd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
        }
        void Socket::setReuseAddr(bool on) {
            int optval = on ? 1 : 0;
            ::setsockopt(sockFd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        }
        void Socket::setReusePort(bool on) {
            int optval = on ? 1 : 0;
            ::setsockopt(sockFd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
        }
        void Socket::setKeepAlive(bool on) {
            int optval = on ? 1 : 0;
            ::setsockopt(sockFd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
        }
    }
}