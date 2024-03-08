#ifndef REACTOREVENTLOOPPOOL_H_
#define REACTOREVENTLOOPPOOL_H_

#include "ReactorEventLoop.h"
#include "thread/threadpool.h"
#include <thread>
#include <vector>
class ReactorEventLoopPool
{
private:
    ReactorEventLoop** loops_;
    int                thread_nums_;
    bool               started_{false};
    int                peek_idx_;

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