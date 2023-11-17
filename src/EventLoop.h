#ifndef EVENTLOOP_H_
#define EVENTLOOP_H_

#include "common/noncopyable.h"
#include <algorithm>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

thread_local const std::thread::id THIS_THREAD_ID = std::this_thread::get_id();

class EventLoop : noncopyable
{
private:
    const std::thread::id loop_thread_id_;

    std::vector<std::function<void()>> pending_functors_;
    std::mutex                         functor_mutex_;

    bool islooping;

public:
    EventLoop();

    void loop();

    template<typename Callable, typename... Args>
    void runInLoop(Callable&& func, Args&&... args);

    void               assetInLoopThread() const;
    [[nodiscard]] bool isInLoopThread() const;

protected:
    virtual void onloop() = 0;
    virtual void wakeUp() = 0;

private:
    void runPendingFunctors();
};

template<typename Callable, typename... Args>
void EventLoop::runInLoop(Callable&& func, Args&&... args)
{
    auto function = std::bind(
        std::forward<Callable>(func),
        std::forward<Args>(args)...);

    if (isInLoopThread())
    {
        function();
        return;
    }

    {
        std::lock_guard<std::mutex> functor_lock(functor_mutex_);
        pending_functors_.push_back(std::move(function));
    }

    wakeUp();
}

#endif  // EVENTLOOP_H_