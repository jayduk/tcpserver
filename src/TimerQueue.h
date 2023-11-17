#ifndef TIMERQUEUE_H_
#define TIMERQUEUE_H_

#include "Poller.h"
#include "TimeStamp.h"
#include "TimerTask.h"

#include <bits/types/struct_timeval.h>
#include <cstdint>
#include <memory>
#include <queue>
#include <sys/time.h>
#include <vector>

class ReactorEventLoop;

class TimerQueue
{
    struct TimerTaskPtrCompare
    {
        bool operator()(const TimerTaskPtr& lhs, const TimerTaskPtr& rhs) const
        {
            return *lhs < *rhs;
        }
    };

    typedef std::priority_queue<TimerTaskPtr, std::vector<TimerTaskPtr>, TimerTaskPtrCompare> TimerTaskPtrMinHeap;

private:
    int timer_fd_;

    TimerTaskPtrMinHeap      timers_;
    std::unique_ptr<Channel> timer_channel_;

public:
    explicit TimerQueue(ReactorEventLoop* loop);

    ~TimerQueue() = default;

    std::vector<TimerTaskPtr> getExpired(TimeStamp now);

    TimerTaskPtr addTimer(TimerTaskPtr timer_task);
    TimerTaskPtr peek();

private:
    static int createTimerFd();

private:
    void handleRead();
    void resetTimerFd();
};

#endif  // TIMERQUEUE_H_