#include "ReactorEventLoop.h"
#include "Channel.h"
#include "EpollPoller.h"
#include "TimerTask.h"
#include "sock.h"
#include <cstdint>
#include <memory>
#include <unistd.h>
#include <utility>

#include "log/easylogging++.h"

ReactorEventLoop::ReactorEventLoop()
  : poller_(new EpollPoller())
  , timer_queue_(new TimerQueue(this))
  , wakeup_fd_(EventFd(0, EFD_NONBLOCK | EFD_CLOEXEC))
  , wakeup_channel_(new Channel(this, wakeup_fd_))
{
    wakeup_channel_->enableReading();

    wakeup_channel_->readableCallback_ = [this] {
        uint64_t result = 0;
        read(wakeup_fd_, &result, result);
    };
}

void ReactorEventLoop::updateChannel(Channel* channel)
{
    runInLoop(&Poller::updateChannel, poller_.get(), channel);
}

void ReactorEventLoop::runAfter(int milliseconds, std::function<void()> callback)
{
    auto task = std::make_shared<TimerTask>(TimeStamp::nowAfter(milliseconds), std::move(callback));
    timer_queue_->addTimer(task);
}

void ReactorEventLoop::runEvery(int interval, std::function<void()> callback, int delay)
{
    auto task = std::make_shared<TimerTask>(TimeStamp::nowAfter(delay), std::move(callback), interval);
    timer_queue_->addTimer(task);
}

void ReactorEventLoop::onloop()
{
    activeChannels_.clear();
    poller_->wait(1000, activeChannels_);

    for (auto&& channel : activeChannels_)
    {
        channel->handleEvent();
    }

    INF << "one loop tick";
}

void ReactorEventLoop::wakeUp()
{
    uint64_t n = 1;
    write(wakeup_fd_, &n, sizeof(uint64_t));
}