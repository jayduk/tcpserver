#ifndef TIMESTAMP_H_
#define TIMESTAMP_H_

#include <bits/types/struct_timespec.h>
#include <bits/types/struct_timeval.h>
#include <cstdint>

class TimeStamp
{
private:
    uint64_t microseconds_;

public:
    explicit TimeStamp(uint64_t microseconds);
    ~TimeStamp() = default;

    static TimeStamp now();
    static TimeStamp nowAfter(uint64_t microseconds);

    bool operator<(const TimeStamp& rhs) const;
    bool operator==(const TimeStamp& rhs) const;

    void addMillis(uint64_t milliseconds);

    [[nodiscard]] uint64_t microseconds() const;
    [[nodiscard]] timespec timespecFromNow() const;
};

#endif  // TIMESTAMP_H_