#include "TimerQueue.h"
#include "TimerTask.h"
#include "tcp/Channel.h"

#include <cassert>
#include <memory>
#include <sys/timerfd.h>
#include <unistd.h>

TimerQueue::TimerQueue(ReactorEventLoop* loop)
  : timer_fd_(createTimerFd())
  , timer_channel_(std::make_unique<Channel>(loop, timer_fd_))
{
    timer_channel_->readableCallback_ = [this] {
        handleRead();
    };

    timer_channel_->enableReading();
}

std::vector<TimerTaskPtr> TimerQueue::getExpired(TimeStamp now)
{
    std::vector<TimerTaskPtr> expired_tasks;
    while (!timers_.empty() && timers_.top()->expired(now)) {
        TimerTaskPtr task = timers_.top();
        timers_.pop();

        if (task->canceled())
            continue;

        expired_tasks.push_back(task);

        if (task->repeat()) {
            task->tick();
            timers_.push(task);
        }
    }

    return expired_tasks;
}

TimerTaskPtr TimerQueue::addTimer(TimerTaskPtr timer_task)
{
    timers_.push(timer_task);

    if (timers_.top() == timer_task)
        resetTimerFd();

    return timer_task;
}

TimerTaskPtr TimerQueue::peek()
{
    while (!timers_.empty()) {
        auto timer_task = timers_.top();
        if (timer_task->canceled())
            timers_.pop();
        else
            return timer_task;
    }

    return nullptr;
}

int TimerQueue::createTimerFd()
{
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timer_fd < 0) {
        perror("timerfd_create");
        abort();
    }
    return timer_fd;
}

void TimerQueue::handleRead()
{
    uint64_t exp;
    ssize_t  n = read(timer_fd_, &exp, sizeof(exp));
    assert(n == sizeof(exp));

    auto expired_tasks = getExpired(TimeStamp::now());
    for (auto& task : expired_tasks)
        task->run();

    resetTimerFd();
}

void TimerQueue::resetTimerFd()
{
    auto timer_task = peek();

    if (!timer_task)
        return;

    struct itimerspec new_value
    {};
    struct itimerspec old_value
    {};

    new_value.it_value = peek()->expiration().timespecFromNow();

    int ret = timerfd_settime(timer_fd_, 0, &new_value, &old_value);
    if (ret) {
        perror("timerfd_settime");
        abort();
    }
}