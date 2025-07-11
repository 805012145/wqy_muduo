#pragma once

#include "Channel.h"
#include "EventLoopThread.h"
#include "noncopyable.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>
namespace mymuduo {
    namespace base {
        class EventLoopThreadPool : noncopyable {
        public:
            using ThreadInitCallback = std::function<void(EventLoop*)>;
            EventLoopThreadPool(EventLoop *baseLoop, const std::string &name);
            ~EventLoopThreadPool();

            void setThreadNum(int numThreads) {numThreads_ = numThreads;}
            void start(const ThreadInitCallback &cb = ThreadInitCallback());
            
            // 多线程环境中，默认以轮询的方式分配Channel给subloop
            EventLoop* getNextLoop();

            bool started() const {return started_;}

            std::vector<EventLoop*> getAllLoops();

            const std::string& name() const {return name_;}

        private:
            std::vector<std::unique_ptr<EventLoopThread>> threads_;
            std::vector<EventLoop*> loops_;
            EventLoop *baseLoop_; // 主Loop
            std::string name_;
            bool started_;
            int numThreads_;
            int next_;
        };
    }
}