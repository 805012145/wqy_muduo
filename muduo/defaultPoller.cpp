#include "Poller.h"
#include "EpollPoller.h"
#include <memory>
#include <stdlib.h>
namespace mymuduo {
    namespace base {
        Poller* Poller::newDefaultPoller(EventLoop* loop) {
            if (::getenv("MUDUO_USE_POLL")) {
                return nullptr;
            } else {
                return new EpollPoller(loop);
            }
        }   
    }
}