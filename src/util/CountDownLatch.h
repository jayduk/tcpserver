#ifndef __COUNTDOWNLATCH_H__
#define __COUNTDOWNLATCH_H__

#include <condition_variable>
#include <cstddef>
#include <mutex>
class CountDownLatch
{
    std::condition_variable cv_;

    std::mutex mt_;
    int        count_;

public:
    explicit CountDownLatch(int count);
    void count_down();
    void wait();
};

CountDownLatch::CountDownLatch(int count)
  : count_(count)
{
}

void CountDownLatch::count_down()
{
    std::unique_lock<std::mutex> lock(mt_);

    if (--count_ <= 0)
    {
        cv_.notify_all();
    }
}

void CountDownLatch::wait()
{
    while (count_ > 0)
    {
        std::unique_lock<std::mutex> lock(mt_);
        cv_.wait(lock);
    }
}

#endif  //_CountDownLatch_h_