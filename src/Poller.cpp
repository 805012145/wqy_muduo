#include "Poller.h"
#include "EventLoop.h"
#include "Channel.h"
#include <memory>

namespace mymuduo {
    namespace base {
        Poller::Poller(EventLoop* loop) : ownerLoop_(loop) {} 

        Poller::~Poller() = default;

        // 判断参数channel是否在当前poller中
        bool Poller::hasChannel(Channel* channel) const {
            auto it = channels_.find(channel->fd());
            return it != channels_.end() && it->second == channel;
        }
    }
}