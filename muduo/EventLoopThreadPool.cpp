#include "EventLoopThreadPool.h"
#include "Channel.h"
#include "EventLoopThread.h"
#include <cstdio>
#include <memory>
#include <mutex>
namespace mymuduo {
    namespace base {
        EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, const std::string &name) 
        : baseLoop_(baseLoop) 
        , name_(name)
        , started_(false)
        , numThreads_(0)
        , next_(0){

        }
        EventLoopThreadPool::~EventLoopThreadPool() {

        }

        void EventLoopThreadPool::start(const ThreadInitCallback &cb) {
            started_ = true;
            for (int i = 0; i < numThreads_; i++) {
                char buf[name_.size() + 32];
                std::snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);
                EventLoopThread *t = new EventLoopThread(cb, buf);
                threads_.push_back(std::unique_ptr<EventLoopThread>(t));
                loops_.push_back(t->startLoop());
            }

            if (numThreads_ == 0 && cb) {
                cb(baseLoop_);
            }
        }
        
        // 多线程环境中，默认以轮询的方式分配Channel给subloop
        EventLoop* EventLoopThreadPool::getNextLoop() {
            EventLoop *loop = baseLoop_;
            if (!loops_.empty()) {
                loop = loops_[next_];
                next_ = (next_ + 1) % loops_.size();
            }
            return loop;
        }


        std::vector<EventLoop*> EventLoopThreadPool::getAllLoops() {
            if (loops_.empty()) {
                return std::vector<EventLoop*>(1, baseLoop_); 
            } else {
                return loops_;
            }
        }
    }
}