#ifndef REACTOREVENTLOOPPOOL_H_
#define REACTOREVENTLOOPPOOL_H_

#include "ReactorEventLoop.h"
#include "thread/threadpool.h"
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
    void makesureStart();
};

#endif  // REACTOREVENTLOOPPOOL_H_