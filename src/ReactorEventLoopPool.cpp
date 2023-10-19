#include "ReactorEventLoopPool.h"
#include "ReactorEventLoop.h"
#include <cassert>
#include <thread>

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
    for (int i = 0; i < thread_nums_; ++i)
    {
        std::thread([this, i] {
            loops_[i] = new ReactorEventLoop();
            loops_[i]->loop();
        }).detach();
    }
}