#include "Poller.h"
#include "EventLoop.h"
#include "Channel.h"
#include <memory>

using namespace mymuduo::base;

Poller::Poller(std::shared_ptr<EventLoop> loop) : ownerLoop_(loop) {} 

Poller::~Poller() = default;

// 判断参数channel是否在当前poller中
bool Poller::hasChannel(std::shared_ptr<Channel> channel) const {
    auto it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}