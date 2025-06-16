#pragma once
#include "Channel.h"
#include "noncopyable.h"
#include "timestamp.h"
#include <memory>
#include <vector>
#include <unordered_map>
namespace mymuduo {
    namespace base {
        class Channel;
        class EventLoop;
        class Poller : noncopyable {
        public:
            using ChannelList = std::vector<Channel*>;
            Poller(EventLoop* loop);
            virtual ~Poller();
            // 给所有IO复用保留统一的接口
            virtual Timestamp poll(int timeoutMs, ChannelList* activateChannels) = 0;
            virtual void updateChannel(Channel* channel) = 0;
            virtual void removeChannel(Channel* channel) = 0;

            // 判断参数channel是否在当前poller中
            bool hasChannel(Channel* channel) const;

            // EventLoop可以通过该接口获取默认的IO复用的具体实现
            static Poller* newDefaultPoller(EventLoop* loop);
        protected:
            using ChannelMap = std::unordered_map<int, Channel*>;
            ChannelMap channels_;
        private:
            EventLoop* ownerLoop_; // poller所属的事件循环
        };
    }
}