#include "EventLoop.h"
#include "Channel.h"
#include "CurrentThread.h"
#include "Poller.h"
#include "logger.h"
#include <bits/stdint-uintn.h>
#include <cerrno>
#include <cstddef>
#include <functional>
#include <mutex>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <unistd.h>

namespace mymuduo {
    namespace base {
        thread_local EventLoop* t_loopInThisThread = nullptr;
        const int kPollTimeMs = 10000;
        int createEventFd() {
            int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
            if (evtfd < 0) {
                LOG_FATAL("eventfd error:%d \n", errno);
            }
            return evtfd;
        }

        EventLoop::EventLoop()
            : looping_(false)
            , quit_(false)
            , callingPendingFunctors_(false)
            , threadId_(CurrentThread::tid())
            , poller_(Poller::newDefaultPoller(this))
            , weakupFd_(createEventFd())
            , weakupChannel_(new Channel(this, weakupFd_))
            , currentActiveChannel_(nullptr)
        {
            LOG_DEBUG("EventLoop created %p int thread %d \n", this, threadId_);
            if (t_loopInThisThread) {
                LOG_FATAL("Another EventLoop %p exists in this thread %d \n", t_loopInThisThread, threadId_);
            } else {
                t_loopInThisThread = this;
            }

            // 设置weakupfd的事件类型以及事件发生后的回调
            weakupChannel_->setReadCallBack(std::bind(&EventLoop::handleRead, this));
            // 每个eventloop都会监听weakupchannel的EPOLLIN读事件
            // todo: (如何确认哪个eventloop对应哪个weakupFd?)
            weakupChannel_->enableReading();
        }

        EventLoop::~EventLoop() {
            weakupChannel_->disableAll();
            weakupChannel_->remove();
            ::close(weakupFd_);
            t_loopInThisThread = nullptr;
        }

        void EventLoop::loop() {
            looping_ = true;
            quit_ = false;

            LOG_INFO("EventLoop %p loop start looping \n", this);
            while (!quit_) {
                activateChannels_.clear();
                pollReturnTime_ = poller_->poll(kPollTimeMs, &activateChannels_);
                for (auto channel : activateChannels_) {
                    channel->handleEvent(pollReturnTime_);
                }
                // 执行当前EventLoop事件循环需要处理的回调操作
                doPendingFunctors();
            }

            LOG_INFO("EventLoop %p loop stop looping \n", this);
            looping_ = false;
        }

        void EventLoop::quit() {
            quit_ = true;
            // 有可能在其他线程中，调用了quit，例如subloop调用mainloop的quit
            if (!isInLoopthread()) {
                weakup();
            }
        }

        void EventLoop::handleRead() {
            uint64_t one = 1;
            ssize_t n = read(weakupFd_, &one, sizeof one);
            if (n != sizeof(one)) {
                LOG_ERROR("EventLoop::handleRead() reads %lu bytes instead of 8", n);
            }
        }

        void EventLoop::runInLoop(Functor cb) {
            if (isInLoopthread()) {
                cb();
            } else {
                queueInLoop(cb);
            }
        }

        void EventLoop::queueInLoop(Functor cb) {
            {
                std::lock_guard<std::mutex> lock(mutex);
                pendingFunctors_.emplace_back(cb);
            }

            // callingPendingFunctors_为true时也要唤醒，因为可能上一轮的回调还没执行完，就又添加了新的回调
            // 此时如果不唤醒的话，loop就会在执行完上一轮回调之后阻塞在poll中，导致新的回调没执行
            // 所以这里需要weakup一下，上一轮执行完，继续处理可读事件，这样就可以调用dopendingfunctors了
            if (!isInLoopthread() || callingPendingFunctors_) {
                weakup(); // 唤醒loop所在线程
            }
        }

        void EventLoop::doPendingFunctors() {
            std::vector<Functor> functors;
            callingPendingFunctors_ = true;
            {
                std::lock_guard<std::mutex> lock(mutex);
                functors.swap(pendingFunctors_);
            }
            for (const auto& func : functors) {
                func();
            }
            callingPendingFunctors_ = false;
        }
        
        bool EventLoop::hasChannel(Channel *channel) {
            return poller_->hasChannel(channel);
        }

        void EventLoop::removeChannel(Channel *channel) {
            poller_->removeChannel(channel);
        }

        void EventLoop::updateChannel(Channel *channel) {
            poller_->updateChannel(channel);
        }

        // 向weakupFd_写数据,weakupChannel就发生读事件，当前loop线程就会被唤醒
        void EventLoop::weakup() {
            uint64_t one = 1;
            size_t n = ::write(weakupFd_, &one, sizeof(one));
            if (n != sizeof(one)) {
                LOG_ERROR("EventLoop::weakup() writes %lu bytes instead of 8 \n", n);
            }
        }
    }
}