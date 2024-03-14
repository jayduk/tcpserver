#include "TimeStamp.h"
#include <cstdint>
#include <sys/time.h>

TimeStamp::TimeStamp(uint64_t microseconds)
  : microseconds_(microseconds)
{}

TimeStamp TimeStamp::now()
{
    struct timeval tv
    {};
    gettimeofday(&tv, nullptr);
    return TimeStamp(tv.tv_sec * 1000000 + tv.tv_usec);
}

TimeStamp TimeStamp::nowAfter(uint64_t microseconds)
{
    TimeStamp ret = TimeStamp::now();
    ret.addMillis(microseconds);
    return ret;
}

bool TimeStamp::operator<(const TimeStamp& rhs) const
{
    return microseconds_ < rhs.microseconds_;
}

bool TimeStamp::operator==(const TimeStamp& rhs) const
{
    return microseconds_ == rhs.microseconds_;
}

void TimeStamp::addMillis(uint64_t milliseconds)
{
    microseconds_ += milliseconds * 1000;
}

uint64_t TimeStamp::microseconds() const
{
    return microseconds_;
}

timespec TimeStamp::timespecFromNow() const
{
    int64_t  microseconds = int64_t(microseconds_ - TimeStamp::now().microseconds());
    timespec ts{};
    ts.tv_sec  = microseconds / 1000000;
    ts.tv_nsec = (microseconds % 1000000) * 1000;
    return ts;
}