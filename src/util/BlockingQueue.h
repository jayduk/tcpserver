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
    std::queue<T>             queue_;
    mutable std::shared_mutex queue_mutex_;
    std::mutex                pop_mutex_;
    std::condition_variable   pop_condition_;

    //TODO: add max size
    int capacity_;

public:
    explicit BlockingQueue(int capacity = 10);
    ~BlockingQueue();

    T    pop();
    bool pop(int millis, T* out);
    void push(T item);
    int  size() const;
    bool empty() const;

    bool try_pop(T* out);
};
template<typename T>
bool BlockingQueue<T>::try_pop(T* out)
{
    std::unique_lock<std::shared_mutex> lock(queue_mutex_);

    if (queue_.empty()) {
        return false;
    }

    (*out) = queue_.front();
    queue_.pop();
    return true;
}

template<typename T>
inline BlockingQueue<T>::BlockingQueue(int capacity)
  : capacity_(capacity)
{
}

template<typename T>
inline BlockingQueue<T>::~BlockingQueue() = default;

template<typename T>
T BlockingQueue<T>::pop()
{
    std::unique_lock<std::mutex> pop_lock(pop_mutex_);

    T ret;
    pop_condition_.wait(pop_lock, [&] {
        return try_pop(&ret);
    });

    return ret;
}

template<typename T>
inline bool BlockingQueue<T>::pop(int millis, T* out)
{
    std::unique_lock<std::mutex> pop_lock(pop_mutex_);

    bool wait_success = pop_condition_.wait_for(
        pop_lock,
        std::chrono::milliseconds(millis),
        [&] {
            return try_pop(out);
        });

    return wait_success;
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
