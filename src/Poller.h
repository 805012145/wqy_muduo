#include "EventLoop.h"
#include "noncopyable.h"
#include "timestamp.h"
#include <memory>
#include <vector>
#include <unordered_map>
namespace mymuduo {
    namespace base {
        class Channel;
        class Poller : noncopyable {
        public:
            using ChannelList = std::vector<std::shared_ptr<Channel>>;
            Poller(std::shared_ptr<EventLoop> loop);
            virtual ~Poller();
            // 给所有IO复用保留同意的接口
            virtual Timestamp poll(int timeoutMs, std::shared_ptr<ChannelList> activateChannels) = 0;
            virtual void updateChannel(std::shared_ptr<Channel> channel) = 0;
            virtual void removeChannel(std::shared_ptr<Channel> channel) = 0;

            // 判断参数channel是否在当前poller中
            bool hasChannel(std::shared_ptr<Channel> channel) const;

            // EventLoop可以通过该接口获取默认的IO复用的具体实现
            static std::shared_ptr<Poller> newDefaultPoller(std::shared_ptr<EventLoop> loop);
        protected:
            using ChannelMap = std::unordered_map<int, std::shared_ptr<Channel>>;
            ChannelMap channels_;
        private:
            std::shared_ptr<EventLoop> ownerLoop_; // poller所属的事件循环
        };
    }
}