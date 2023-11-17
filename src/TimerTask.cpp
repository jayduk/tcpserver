#include "TimerTask.h"
#include <cassert>
#include <utility>

TimerTask::TimerTask(TimeStamp expiration, std::function<void()> callback)
  : TimerTask(expiration, std::move(callback), 0)
{
}

TimerTask::TimerTask(TimeStamp expiration, std::function<void()> callback, int interval)
  : repeat_(interval > 0)
  , canceled_(false)
  , interval_(interval)
  , expiration_(expiration)
  , callback_(std::move(callback))
{
}

bool TimerTask::expired(TimeStamp time) const
{
    return expiration_ < time;
}

bool TimerTask::canceled() const
{
    return canceled_;
}

bool TimerTask::repeat() const
{
    return repeat_;
}

TimeStamp TimerTask::expiration() const
{
    return expiration_;
}

void TimerTask::run()
{
    callback_();
}

void TimerTask::tick()
{
    assert(repeat_ && !canceled_);

    expiration_.addMillis(interval_);
}

bool TimerTask::operator<(const TimerTask& rhs) const
{
    return expiration_ < rhs.expiration_;
}