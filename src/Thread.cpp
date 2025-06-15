#include "Thread.h"
#include "CurrentThread.h"
#include <algorithm>
#include <cstdio>
#include <memory>
#include <thread>
#include <semaphore.h>

namespace mymuduo {
    namespace base {
        std::atomic_int Thread::numCreated_(0);
        Thread::Thread(ThreadFunc func, const std::string& name)
        : started_(false)
        , joined_(false)
        , tid_ (0)
        , func_(std::move(func))
        , name_(name) {
            setDefaultName();
        }
        Thread::~Thread() {
            if (started_ && !joined_) {
                thread_->detach();
            }
        }
        void Thread::start() { // 一个Thread对象，就是一个新线程的详细信息
            started_ = true;
            sem_t sem;
            sem_init(&sem, 0, 0);
            thread_ = std::make_shared<std::thread>([&]() {
                tid_ = mymuduo::CurrentThread::tid();
                sem_post(&sem);
                func_();
            });
            sem_wait(&sem); 
        }
        void Thread::join() {
            joined_ = true;
            thread_->join();
        }
        void Thread::setDefaultName() {
            int number = ++numCreated_;
            if (name_.empty()) {
                char buf[32] = {0};
                snprintf(buf, sizeof(buf), "Thread%d", number);
                name_ = buf;
            }
        }
    }
}