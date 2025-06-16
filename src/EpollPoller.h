#pragma once
#include "EventLoop.h"
#include "Poller.h"
#include <vector>
#include <sys/epoll.h>
#include <memory>

/**
    epoll_create
    epoll_ctl (add/mod/del)
    epoll_wait
 */
namespace mymuduo {
    namespace base {
        class EpollPoller : public Poller {
        public:
            EpollPoller(EventLoop* loop);
            ~EpollPoller() override; // override 可以让编译器帮我检查基类里是否为虚函数

            Timestamp poll(int timeoutMs, ChannelList* activateChannels) override;
            void updateChannel(Channel* channel) override;
            void removeChannel(Channel* channel) override;
            
        private:
            static const int kInitEventsListSize = 16;

            // 填写活跃连接
            void fillActivateChannels(int numEvents, ChannelList* activateChannels) const;
            // 调用epoll_ctl 更新channel
            void update(int operation, Channel* channel);

            using EventList = std::vector<struct epoll_event>;
            EventList events_; // epoll_wait返回发生的事件的fd
            int epoll_fd_; // epoll_create返回值
        };
    }
}