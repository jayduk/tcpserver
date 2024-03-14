#include "ThreadPool.h"
#include <iostream>

ThreadPool::ThreadPool(uint16_t thread_size, uint16_t thread_max_size, int thread_wait_millis)
  : thread_default_size_(thread_size)
  , thread_max_size_(thread_max_size)
  , thread_wait_millis_(thread_wait_millis)
  , shutdown_(false)
  , thread_count_(0)
  , available_count_(0)
{}

void ThreadPool::waitAll()
{
    if (tasks_.empty() && thread_count_ == available_count_)
        return;

    std::unique_lock<std::mutex> remove_lock(remove_mutex_);

    available_cv_.wait(remove_lock, [&] {
        return tasks_.empty() && thread_count_ == available_count_;
    });
}

void ThreadPool::shutdown()
{
    shutdown_ = true;
    while (thread_count_ > 0) {
        removeThread();
    }
}

void ThreadPool::createThread()
{
    thread_count_++;
    available_count_++;
    auto thread = std::thread(&ThreadPool::threadFunc, this);
    thread.detach();
}

void ThreadPool::removeThread()
{
    std::unique_lock<std::mutex> remove_lock(remove_mutex_);
    tasks_.push(nullptr);
}

void ThreadPool::threadFunc()
{
    while (!shutdown_) {
        std::function<void()> func;

        if (!tasks_.pop(thread_wait_millis_, &func)) {
            if (thread_count_ > thread_default_size_) {
                break;
            } else {
                continue;
            }
        }

        if (func == nullptr) {
            break;
        }

        available_count_--;
        func();
        available_count_++;

        available_cv_.notify_all();
    }

    thread_count_--;
    available_count_--;

    available_cv_.notify_all();
}