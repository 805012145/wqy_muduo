#include "EpollPoller.h"
#include "Poller.h"
#include "logger.h"
#include "Channel.h"
#include "timestamp.h"

#include <asm-generic/errno-base.h>
#include <cerrno>
#include <strings.h>
#include <sys/epoll.h>
#include <unistd.h>

namespace mymuduo {
    namespace base {
        const int kNew = -1;
        const int kAdd = 1;
        const int kDelete = 2;

        EpollPoller::EpollPoller(EventLoop* loop)
            : Poller(loop)
            , epoll_fd_(::epoll_create1(EPOLL_CLOEXEC))
            , events_(kInitEventsListSize) 
        {
            if (epoll_fd_ < 0) {
                LOG_FATAL("epoll_create error:%d \n", errno);
            }
        }

        EpollPoller::~EpollPoller() {
            ::close(epoll_fd_);

        }

        Timestamp EpollPoller::poll(int timeoutMs, ChannelList* activateChannels) {
            LOG_INFO("func=%s => fd total count:%ld \n", __FUNCTION__, channels_.size());
            int numEvents = ::epoll_wait(epoll_fd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
            int saveErrno = errno;
            Timestamp now(Timestamp::now());
            if (numEvents > 0) {
                LOG_INFO("%d events happend \n", numEvents);
                fillActivateChannels(numEvents, activateChannels);
                // 发生事件的events个数 与 events_大小相同，则需要扩容
                if (numEvents == events_.size()) {
                    events_.resize(events_.size() * 2);
                }
            } else if(numEvents == 0) {
                LOG_DEBUG("%s timeout! \n", __FUNCTION__);
            } else {
                if (saveErrno != EINTR) {
                    errno = saveErrno;
                    LOG_ERROR("EPoller::poll() err!");
                }
            }
            return now;
        }
        void EpollPoller::updateChannel(Channel* channel) {
            int index = channel->index();
            LOG_INFO("func=%s => fd=%d events=%d index=%d \n", __FUNCTION__, channel->fd(), channel->events(), index);
            if (index == kNew || index == kDelete) {
                if (index == kNew) {
                    int fd = channel->fd();
                    channels_[fd] = channel;
                }
                channel->set_index(kAdd);
                update(EPOLL_CTL_ADD, channel);
            } else {
                int fd = channel->fd();
                if (channel->isNoneEvent()) {
                    update(EPOLL_CTL_DEL, channel);
                    channel->set_index(kDelete);
                } else {
                    update(EPOLL_CTL_MOD, channel);
                }
            }
        }
        void EpollPoller::removeChannel(Channel* channel) {
            int fd = channel->fd();
            channels_.erase(fd);
            LOG_INFO("func=%s => fd=%d \n", __FUNCTION__, fd);    
            int index = channel->index();
            // 如果index是kAdd状态，则需要在epoll删除;
            // kDelete状态则不需要，因为updateChannel的时候已经删除过了。
            if (index == kAdd) {
                update(EPOLL_CTL_DEL, channel);
            }
            channel->set_index(kNew);
        }

        // 填写活跃连接
        void EpollPoller::fillActivateChannels(int numEvents, ChannelList* activateChannels) const {
            for (const auto &event : events_) {
                Channel* channel = static_cast<Channel*>(event.data.ptr);
                channel->set_revents(event.events);
                activateChannels->emplace_back(channel);
            }
        }
        // 调用epoll_ctl 更新channel
        void EpollPoller::update(int operation, Channel* channel) {
            struct epoll_event event;
            bzero(&event, sizeof event);
            int fd = channel->fd();
            event.events = channel->events();
            event.data.fd = fd;
            event.data.ptr = channel;
            
            if (::epoll_ctl(epoll_fd_, operation, fd, &event) < 0) {
                if (operation == EPOLL_CTL_DEL) {
                    LOG_ERROR("epoll_ctl del error:%d \n", errno);
                } else {
                    LOG_FATAL("epoll_ctl del error:%d \n", errno);
                }
            }
        }
    }
}