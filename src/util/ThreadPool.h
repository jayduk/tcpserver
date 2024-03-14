#ifndef __NET_THREAD_POOL_H__
#define __NET_THREAD_POOL_H__

#include "BlockingQueue.h"

#include <functional>
#include <future>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool
{
private:
    uint16_t thread_default_size_;
    uint16_t thread_max_size_;
    int      thread_wait_millis_;

    std::atomic<bool>                    shutdown_;
    std::atomic<int>                     thread_count_;
    std::atomic<int>                     available_count_;
    BlockingQueue<std::function<void()>> tasks_;

    std::condition_variable available_cv_;
    std::mutex              remove_mutex_;

public:
    explicit ThreadPool(uint16_t thread_size = 64, uint16_t thread_max_size = 128, int thread_wait_millis = 5000);
    ~ThreadPool() = default;

    std::string info() const
    {
        return "total: " + std::to_string(thread_count_) + ", " +
               "available: " + std::to_string(available_count_);
    }

public:
    template<typename Callable, typename... Args>
    auto commit(Callable&& task, Args&&... args) -> std::future<decltype(std::bind(
                                                     std::forward<Callable>(task),
                                                     std::forward<Args>(args)...)())>;

    void waitAll();
    // TODO: 待完善
    void shutdown();

private:
    void createThread();
    void removeThread();
    void threadFunc();
};

template<typename Callable, typename... Args>
auto ThreadPool::commit(Callable&& task, Args&&... args)
    -> std::future<decltype(std::bind(
        std::forward<Callable>(task),
        std::forward<Args>(args)...)())>
{
    auto func = std::bind(
        std::forward<Callable>(task),
        std::forward<Args>(args)...);

    using RetType = decltype(func());

    // Encapsulate it into a shared pointer in order to be able to copy construct
    auto task_ptr = std::make_shared<std::packaged_task<RetType(void)>>(func);

    // Warp packaged task into void function
    std::function<void()> wrapper_func = [task_ptr]() {
        (*task_ptr)();
    };

    // 队列通用安全封包函数，并压入阻塞队列
    tasks_.push(wrapper_func);

    if (tasks_.size() > available_count_ && thread_count_ < thread_max_size_) {
        createThread();
    }

    // 返回先前注册的任务指针
    return task_ptr->get_future();
}

#endif  //_thread pool_h_