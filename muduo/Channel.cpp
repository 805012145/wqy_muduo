#include "Channel.h"
#include "EventLoop.h"
#include "logger.h"
#include <memory>
#include <sys/epoll.h>

namespace mymuduo {
    namespace base {
        const int Channel::kNoneEvent = 0x00;
        const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
        const int Channel::kWriteEvent= EPOLLOUT | EPOLLPRI;

        Channel::Channel(EventLoop* loop, int fd)
        : loop_(loop), 
        fd_(fd), 
        events_(0), 
        revents_(0),
        index_(-1),
        tied_(false) {

        }
        Channel::~Channel() {

        }

        // fd被通知后，处理事件
        void Channel::handleEvent(Timestamp receiveTime) {
            std::shared_ptr<void> guard;
            if (tied_) {
                guard = tie_.lock();
                if (guard) {
                    handleEventWithGuard(receiveTime);
                }
            } else {
                handleEventWithGuard(receiveTime);
            }
        }

        void Channel::handleEventWithGuard(Timestamp receiveTime) {
            LOG_INFO("channel handleEvent revents: %d", revents_);
            if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
                if (!!closeCallBack_) {
                    closeCallBack_();
                }
            } 
            if (revents_ & EPOLLERR) {
                if (!!errorCallBack_) {
                    errorCallBack_();
                }
            }
            if (revents_ & EPOLLIN) {
                if (!!readCallBack_) {
                    readCallBack_(receiveTime);
                }
            }
            if (revents_ & EPOLLOUT) {
                if (!!writeCallBack_) {
                    writeCallBack_();
                }
            }
        }

        // 防止当channel被手动remove掉，channel还在执行回调操作
        void Channel::tie(const std::shared_ptr<void>& obj) {
            tie_ = obj;
            tied_ = true;
        }

        // 当改变channel所表示的fd的events事件后，update负责在poller里更改fd相应的epoll_ctl
        void Channel::update() {
            // 创建一个 shared_ptr 指向当前对象
            loop_->updateChannel(this);
        }

        void Channel::remove() {
            loop_->removeChannel(this);
        }
    }
}