#include "EpollPoller.h"
#include "Channel.h"
#include "sock.h"
#include <cstring>
#include <sys/epoll.h>
#include <utility>
#include <vector>

#include "log/easylogging++.h"

EpollPoller::EpollPoller(int poller_size)
  : epoll_fd_(Epoll_create1(0))
  , events_(poller_size)
{
}

EpollPoller::~EpollPoller() = default;

void EpollPoller::wait(int wait_millis, std::vector<Channel*>& activeChannel)
{
    int nfd = Epoll_wait(epoll_fd_, &*events_.begin(), static_cast<int>(events_.size()), wait_millis);

    for (int i = 0; i < nfd; ++i)
    {
        auto channel = (Channel*)events_[i].data.ptr;
        channel->set_revent(events_[i].events);
        activeChannel.push_back(channel);
    }

    if (nfd == static_cast<int>(events_.size()))
        events_.resize(2 * nfd);
}

void EpollPoller::updateChannel(Channel* channel)
{
    if (channel->index() == Channel::kToDel)
    {
        INF << channel->fd() << " DEL from reactorloop";
        Epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, channel->fd(), nullptr);
        return;
    }

    epoll_event event{};
    event.events = channel->event();
    event.data.ptr = channel;

    if (channel->index() == Channel::kToAdd)
        Epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, channel->fd(), &event);
    else
        Epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, channel->fd(), &event);
}