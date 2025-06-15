#pragma once

#include "Channel.h"
#include "InetAddress.h"
#include "Socket.h"
#include <functional>
namespace mymuduo {
    namespace base {
        class Acceptor {
        public:
            using NewConnectionCallback = std::function<void(int sockFd, const InetAddress&)>;
            // 用户定义的baseLoop(mainLoop)
            Acceptor(EventLoop *loop, const InetAddress& addr, bool reusePort);
            ~Acceptor();

            void setNewConnectionCallback(const NewConnectionCallback &cb) {
                newConnectionCallback_ = std::move(cb);
            }
            bool listening() const {return listening_;}
            void listen();

        private:
            void handleRead();
            EventLoop *loop_;
            Socket acceptSocket_;
            Channel acceptChannel_;
            NewConnectionCallback newConnectionCallback_;
            bool listening_;
            int idleFd_;
        };
    }
}