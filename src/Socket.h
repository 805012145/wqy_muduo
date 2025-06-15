#pragma once

#include "noncopyable.h"
namespace mymuduo {
    namespace base {
        class InetAddress;
        class Socket : noncopyable {
        public:
            explicit Socket(int sockFd) : sockFd_(sockFd) {}
            ~Socket();

            int fd() const {return sockFd_;}
            void bindAddress(const InetAddress& localAddr);
            void listen();
            int accept(InetAddress *peerAddr);

            void shutdownWrite();

            void setTcpNoDelay(bool on);
            void setReuseAddr(bool on);
            void setReusePort(bool on);
            void setKeepAlive(bool on);

        private:
            const int sockFd_;
        };
    }
}