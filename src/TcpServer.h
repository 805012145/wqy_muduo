#pragma once
#include "Channel.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"
#include "InetAddress.h"
#include "noncopyable.h"
#include "Callbacks.h"
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
namespace mymuduo {
    namespace base {
        class TcpServer : noncopyable {
        public:
            using ThreadInitCallback = std::function<void(EventLoop*)>;
            
            enum Option {
                kNoReusePort,
                kReusePort,
            };

            TcpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &nameArg, Option option = kNoReusePort);
            ~TcpServer();

            void setThreadInitcallback(const ThreadInitCallback &cb) {threadInitCallback_ = cb;}
            void setConnectionCallback(const ConnectionCallback &cb) {connectionCallback_ = cb;}
            void setMessageCallback(const MessageCallback &cb) {messageCallback_ = cb;}
            void setWriteCompleteCallback(const WriteCompleteCallback &cb) {writeCompleteCallback_ = cb;}

            // 设置subLoop的个数
            void setThreadNum(int numThreads) {threadPool_->setThreadNum(numThreads);}

            // 开启服务器监听(开启listen)
            void start();
            
        private:
            void newConnection(int sockFd, const InetAddress &peerAddr);
            void removeConnection(const TcpConnectionPtr &conn);
            void removeConnectionInLoop(const TcpConnectionPtr &conn);
            
            using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;
            
            EventLoop *loop_; // mainLoop

            const std::string ipPort_; // 服务器ip:port
            const std::string name_; // 服务器名
            
            std::unique_ptr<Acceptor> acceptor_; // 运行在mainLoop，监听新连接事件
            std::shared_ptr<EventLoopThreadPool> threadPool_; // subLoops
            
            ConnectionCallback connectionCallback_; // 有新连接回调
            MessageCallback messageCallback_; // 有读写消息的回调
            WriteCompleteCallback writeCompleteCallback_; // 消息发送完成的回调函数

            ThreadInitCallback threadInitCallback_; // loop线程初始化的回调
            std::atomic_int started_;

            int nextConnId_;
            ConnectionMap connections_; // 保存所有的连接

        };
    }
}