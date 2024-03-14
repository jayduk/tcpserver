#ifndef TIMERTASK_H_
#define TIMERTASK_H_

#include "TimeStamp.h"
#include <bits/types/struct_timespec.h>
#include <functional>
#include <memory>

class TimerTask
{
private:
    bool repeat_;
    bool canceled_;
    int  interval_;

    TimeStamp expiration_;

    std::function<void()> callback_;

public:
    TimerTask(TimeStamp expiration, std::function<void()> callback);
    TimerTask(TimeStamp expiration, std::function<void()> callback, int interval);

    ~TimerTask() = default;

    [[nodiscard]] bool expired(TimeStamp time) const;
    [[nodiscard]] bool canceled() const;
    [[nodiscard]] bool repeat() const;

    [[nodiscard]] TimeStamp expiration() const;

    void run();

    void tick();

    bool operator<(const TimerTask& rhs) const;
};

typedef std::shared_ptr<TimerTask> TimerTaskPtr;

#endif  // TIMERTASK_H_