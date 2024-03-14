#ifndef CHANNEL_H_
#define CHANNEL_H_

#include "ReactorEventLoop.h"
#include "common/noncopyable.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <sys/epoll.h>
#include <utility>

class Channel : noncopyable
{
public:
    constexpr static int kToAdd = 0;
    constexpr static int kToMod = 1;
    constexpr static int kToDel = 2;

private:
    ReactorEventLoop* loop_;
    int               index_;  // 0->add, 1->delete, 2->update
    int               sockfd_;
    uint32_t          event_;
    uint32_t          revent_;

    std::weak_ptr<void> tie_;
    bool                tied_;

public:
    Channel(ReactorEventLoop* loop, int fd);
    ~Channel() override;

public:
    std::function<void(void)> readableCallback_;
    std::function<void(void)> writeableCallback_;
    std::function<void(void)> closeCallback_;

public:
    void handleEvent();

    void tie(const std::shared_ptr<void>& t);

    void enableReading();
    void enableWriting();
    void disableWriting();

    [[nodiscard]] bool isWriting() const;

    [[nodiscard]] int fd() const;

    [[nodiscard]] uint32_t event() const;

    [[nodiscard]] int index() const;

    void set_index(int index);

    void set_revent(uint32_t event);
    void close();

private:
    [[nodiscard]] std::string eventToString() const;

    void handleEventWithGuard();
};

#endif  // CHANNEL_H_