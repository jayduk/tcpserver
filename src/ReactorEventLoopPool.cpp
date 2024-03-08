#include "ReactorEventLoopPool.h"
#include "ReactorEventLoop.h"
#include "util/CountDownLatch.h"
#include <cassert>
#include <thread>

#include "log/easylogging++.h"

void ReactorEventLoopPool::setThreadNum(int num)
{
    thread_nums_ = num;
}

ReactorEventLoop* ReactorEventLoopPool::selectLoop()
{
    makesureStart();
    peek_idx_ = (peek_idx_ + 1) % thread_nums_;
    return loops_[peek_idx_];
}

void ReactorEventLoopPool::makesureStart()
{
    if (started_)
        return;

    assert(loops_ == nullptr);
    started_ = true;

    loops_ = new ReactorEventLoop*[thread_nums_];
    CountDownLatch latch(thread_nums_);
    for (int i = 0; i < thread_nums_; ++i)
    {
        std::thread([this, i, &latch] {
            loops_[i] = new ReactorEventLoop();
            latch.count_down();
            loops_[i]->loop();
        }).detach();
    }

    INF << "wait...";
    latch.wait();
    INF << "end wait";
}