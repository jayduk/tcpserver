#ifndef REACTOREVENTLOOPPOOL_H_
#define REACTOREVENTLOOPPOOL_H_

#include "ReactorEventLoop.h"
#include "util/ThreadPool.h"
#include <cstddef>
#include <thread>
#include <vector>
class ReactorEventLoopPool
{
private:
    ReactorEventLoop** loops_{nullptr};
    int                thread_nums_{1};
    bool               started_{false};
    int                peek_idx_{0};

public:
    ReactorEventLoopPool()  = default;
    ~ReactorEventLoopPool() = default;

public:
    void              setThreadNum(int num);
    ReactorEventLoop* selectLoop();

private:
    void makeSureStart();
};

#endif  // REACTOREVENTLOOPPOOL_H_