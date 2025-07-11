#pragma once
#include "noncopyable.h"
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "timestamp.h"
#include <atomic>
#include <cstddef>
#include <memory>
#include <string>
namespace mymuduo {
    namespace base {
        class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection> {
        public:
            TcpConnection(EventLoop *loop, 
                const std::string &name, 
                int sockfd, 
                const InetAddress &localAddr, 
                const InetAddress &peerAddr);
            ~TcpConnection();

            EventLoop *getLoop() const { return loop_; } // 获取事件循环
            const std::string &name() const { return name_; } // 获取连接名称
            const InetAddress &localAddress() const { return localAddr_; } // 获取本地
            const InetAddress &peerAddress() const { return peerAddr_; } // 获取对端地址

            bool connected() const {return state_ == kConnected;}
            bool disconnected() const {return state_ == kDisconnected;}
            
            void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; } // 设置连接回调
            void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
            void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; } // 设置写完成回调
            void setCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }
            void setHighWaterMarkCallback(const HighWaterMarkCallback &cb, size_t highWaterMark) {
                highWaterMarkCallback_ = cb; 
                highWaterMark_ = highWaterMark; 
            }

            void send(const std::string &buf); // 发送数据

            void conneectEstableished(); // 连接建立
            void conneectDestroyed(); // 连接销毁

            void shutdown(); // 关闭连接

        private:
            enum StateE {
                kDisconnected, // 断开连接
                kConnecting,   // 正在连接
                kConnected,    // 已连接
                kDisconnecting // 正在断开连接
            };
            void setState(StateE state) { state_ = state; } // 设置连接状态

            void handleRead(Timestamp receiveTime); // 处理读取事件
            void handleWrite(); // 处理写事件
            void handleClose(); // 处理关闭事件
            void handleError(); // 处理错误事件


            void sendInLoop(const void *message, int len); // 在事件循环中发送数据
            void shutdownInLoop(); // 在事件循环中关闭连接


            EventLoop *loop_; //管理该连接的事件循环
            std::string name_; //连接的名称
            std::atomic_int state_; //连接状态
            bool reading_;

            std::unique_ptr<Socket> socket_; //套接字对象
            std::unique_ptr<Channel> channel_; //通道对象

            const InetAddress localAddr_; //本地地址
            const InetAddress peerAddr_; //本地地址

            ConnectionCallback connectionCallback_; //连接回调函数
            MessageCallback messageCallback_; //消息回调函数
            WriteCompleteCallback writeCompleteCallback_; //写完成回调函数
            CloseCallback closeCallback_; //关闭回调函数
            HighWaterMarkCallback highWaterMarkCallback_; //高水位回调函数

            size_t highWaterMark_;

            Buffer inputBuffer_; //输入缓冲区
            Buffer outputBuffer_; //输出缓冲区
        };
    }
}