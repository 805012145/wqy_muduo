#include "Poller.h"
#include <memory>
#include <stdlib.h>
using namespace mymuduo::base;


std::shared_ptr<Poller> Poller::newDefaultPoller(std::shared_ptr<EventLoop> loop) {
    if (::getenv("MUDUO_USE_POLL")) {
        return nullptr;
        // return std::make_shared<PollPoller>();
    } else {
        return nullptr;
        // return std::make_shared<EPollPoller>();
    }
}