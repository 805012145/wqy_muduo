#pragma once
#include <functional>
#include <memory>

#include "noncopyable.h"
#include "timestamp.h"

namespace mymuduo {
    namespace base {
        class EventLoop;

        class Channel : noncopyable {
        public:
            typedef std::function<void()> EventCallback;
            typedef std::function<void(Timestamp)> ReadEventCallback;

            explicit Channel(EventLoop* loop, int fd);
            ~Channel();
            
            // fd被通知后，处理事件
            void handleEvent(Timestamp receiveTime);

            // 设置回调函数
            void setReadCallBack(ReadEventCallback cb) {readCallBack_ = std::move(cb);}
            void setWriteCallBack(EventCallback cb) {writeCallBack_ = std::move(cb);}
            void setCloseCallBack(EventCallback cb) {closeCallBack_ = std::move(cb);}
            void setErrorCallBack(EventCallback cb) {errorCallBack_ = std::move(cb);}

            // 防止当channel被手动remove掉，channel还在执行回调操作
            void tie(const std::shared_ptr<void>&);

            int fd() const {return fd_;}
            int events() const {return events_;}

            // epoll监听事件后，设置Channel具体发生的事件
            void set_revents(int revt) {revents_ = revt;}

            void enableReading() {
                events_ |= kReadEvent; // 0000 | 0001 = 0001
                update();
            }
            void disableReading() {
                events_ &= ~kReadEvent; // 0000 & 1110 = 0000
                update();
            }
            void enableWriting() {
                events_ |= kWriteEvent; // 0000 | 0010 = 0010
                update();
            }
            void disableWriting() {
                events_ &= ~kWriteEvent; // 0000 & 1101 = 0000
                update();
            }
            void disableAll() { 
                events_ = kNoneEvent;  // 0000
                update(); 
            }

            // 返回fd当前的事件状态
            bool isNoneEvent() const { return events_ == kNoneEvent; }
            bool isWriting() const { return events_ & kWriteEvent; }
            bool isReading() const { return events_ & kReadEvent; }
        
            int index() { return index_; }
            void set_index(int idx) { index_ = idx; }

            EventLoop* ownerLoop() { return loop_; }
            void remove();
        private:

            void update();
            void handleEventWithGuard(Timestamp receiveTime);

            static const int kNoneEvent;
            static const int kReadEvent;
            static const int kWriteEvent;
            
            const int fd_;
            EventLoop* loop_;
            int events_;    // 感兴趣的事件
            int revents_;   // 具体发生的事件
            int index_;     

            std::weak_ptr<void> tie_;
            bool tied_;

            EventCallback writeCallBack_;
            EventCallback closeCallBack_;
            EventCallback errorCallBack_;
            ReadEventCallback readCallBack_;
        };
    }
}