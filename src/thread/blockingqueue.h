#ifndef __BLOCKING_QUEUE_H__
#define __BLOCKING_QUEUE_H__

#include <condition_variable>
#include <mutex>
#include <queue>
#include <shared_mutex>

template<typename T>
class BlockingQueue
{
private:
    std::queue<T> queue_;
    mutable std::shared_mutex queue_mutex_;
    std::mutex pop_mutex_;
    std::condition_variable pop_condition_;

public:
    BlockingQueue();
    ~BlockingQueue();

    T pop();
    bool pop(int millis, T* out);
    void push(T item);
    int size() const;
    bool empty() const;
};

template<typename T>
inline BlockingQueue<T>::BlockingQueue() = default;

template<typename T>
inline BlockingQueue<T>::~BlockingQueue() = default;

template<typename T>
T BlockingQueue<T>::pop()
{
    std::unique_lock<std::mutex> pop_lock(pop_mutex_);
    pop_condition_.wait(pop_lock, [&] {
        return this->size() != 0;
    });

    std::unique_lock<std::shared_mutex> lock(queue_mutex_);
    T ret = queue_.front();
    queue_.pop();
    return ret;
}

template<typename T>
inline bool BlockingQueue<T>::pop(int millis, T* out)
{
    std::unique_lock<std::mutex> pop_lock(pop_mutex_);
    bool is_overtime = !pop_condition_.wait_for(
        pop_lock,
        std::chrono::milliseconds(millis),
        [&] {
            return this->size() != 0;
        });
    if (is_overtime)
    {
        return false;
    }

    std::unique_lock<std::shared_mutex> lock(queue_mutex_);
    (*out) = queue_.front();
    queue_.pop();
    return true;
}

template<typename T>
void BlockingQueue<T>::push(T item)
{
    std::unique_lock<std::shared_mutex> lock(queue_mutex_);
    queue_.push(item);
    pop_condition_.notify_one();
}

template<typename T>
int BlockingQueue<T>::size() const
{
    std::shared_lock<std::shared_mutex> lock(queue_mutex_);
    return queue_.size();
}

template<typename T>
inline bool BlockingQueue<T>::empty() const
{
    return size() == 0;
}

#endif  //_block queue_h_
