#include "EventLoop.h"
#include <cassert>
#include <functional>
#include <iostream>
#include <thread>
#include <vector>

#include "log/easylogging++.h"

EventLoop::EventLoop()
  : loop_thread_id_(std::this_thread::get_id())
  , islooping(false)
{
}

void EventLoop::start()
{
}

void EventLoop::loop()
{
    assetInLoopThread();

    islooping = true;
    while (true)
    {
        onloop();
        runPendingFunctors();
    }

    islooping = false;
}

void EventLoop::assetInLoopThread() const
{
    assert(loop_thread_id_ == THIS_THREAD_ID);
}

bool EventLoop::isInLoopThread() const
{
    return loop_thread_id_ == THIS_THREAD_ID;
}

void EventLoop::runPendingFunctors()
{
    if (pending_functors_.empty())
        return;

    std::vector<std::function<void()>> functors;

    {
        std::lock_guard<std::mutex> functor_lock(functor_mutex_);
        pending_functors_.swap(functors);
    }

    for (auto&& function : functors)
        function();
}