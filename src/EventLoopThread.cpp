#include "EventLoopThread.h"
#include "EventLoop.h"
#include <functional>
#include <mutex>

namespace mymuduo {
    namespace base {
        EventLoopThread::EventLoopThread(const ThreadInitCallback &cb, 
            const std::string &name) 
            : loop_(nullptr)
            , thread_(std::bind(&EventLoopThread::threadFunc, this), name)
            , mutex_()
            , cond_()
            , exiting_(false)
            , callback_(cb){
            
        }
        EventLoopThread::~EventLoopThread() {
            exiting_ = true;
            if (loop_ != nullptr) {
                loop_->quit();
                thread_.join();
            }
        }

        EventLoop* EventLoopThread::startLoop() {
            thread_.start();
            EventLoop *loop = nullptr;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                while (loop_ == nullptr) {
                    cond_.wait(lock);
                }
                loop = loop_;
            }
            return loop;
        }
        void EventLoopThread::threadFunc() {
            EventLoop loop;
            if (callback_) {
                callback_(&loop);
            }

            {
                std::unique_lock<std::mutex> lock(mutex_);
                loop_ = &loop;
                cond_.notify_one();
            }

            loop.loop();
            // loop.loop()退出后，loop_置空
            std::unique_lock<std::mutex> lock(mutex_);
            loop_ = nullptr;
        }
    }
}