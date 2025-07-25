#pragma once
#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <sched.h>
#include <vector>
#include <mutex>

#include "Channel.h"
#include "CurrentThread.h"
#include "Poller.h"
#include "noncopyable.h"
#include "timestamp.h"
namespace mymuduo {
    namespace base {
        class EventLoop : noncopyable {
        public:
            using Functor = std::function<void()>;
            EventLoop();
            ~EventLoop();

            // 开启事件循环
            void loop();
            // 退出事件循环
            void quit();

            Timestamp pollReturnTime() const {return pollReturnTime_;}

            // 在当前loop中执行cb
            void runInLoop(Functor cb);
            // 把cb放入队列，唤醒Loop所在的线程执行cb
            void queueInLoop(Functor cb);

            // 唤醒loop所在的线程
            void weakup();

            void updateChannel(Channel *channel);
            void removeChannel(Channel *channel);
            bool hasChannel(Channel *channel);
            
            // 判断Eventloop对象是否在自己的线程里面
            bool isInLoopthread() const { return threadId_ == CurrentThread::tid();}
        private:
            // weak up
            void handleRead();
            // 执行回调
            void doPendingFunctors();
            
            using ChannelList = std::vector<Channel*>;
            
            std::atomic<bool> looping_; // 原子操作，底层通过cas实现
            std::atomic<bool> quit_;
            const pid_t threadId_; // 记录当前loop所在的线程id
            Timestamp pollReturnTime_; // poller返回发生事件的channels的时间点
            std::unique_ptr<Poller> poller_;
            int weakupFd_; // 当mainLoop获取一个新用户的channel, 通过轮询算法选择一个subloop, 通过该成员唤醒subloop
            std::unique_ptr<Channel> weakupChannel_;

            ChannelList activateChannels_;
            Channel* currentActiveChannel_;

            std::atomic<bool> callingPendingFunctors_; // 标识当前loop是否有需要执行的回调操作
            std::vector<Functor> pendingFunctors_; // 存储loop需要执行的所有回调操作
            std::mutex mutex; // 互斥锁，用来保护pendingFunctors_的线程安全操作
        };
    }
}