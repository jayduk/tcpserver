#ifndef REACTOREVENTLOOP_H_
#define REACTOREVENTLOOP_H_

#include "Poller.h"
#include "TimerQueue.h"
#include "tcp/EventLoop.h"
#include <memory>
#include <vector>

class Channel;

class ReactorEventLoop : public EventLoop
{
private:
    std::unique_ptr<Poller>     poller_;
    std::unique_ptr<TimerQueue> timer_queue_;

    std::vector<Channel*> ownChannels_;
    std::vector<Channel*> activeChannels_;

    int      wakeup_fd_;
    Channel* wakeup_channel_;

public:
    ReactorEventLoop();
    ~ReactorEventLoop() override = default;

    void updateChannel(Channel* channel);
    void runAfter(int milliseconds, std::function<void()> callback);
    void runEvery(int interval, std::function<void()> callback, int delay = 0);

protected:
    void onloop() override;
    void wakeUp() override;
};

#endif  // REACTOREVENTLOOP_H_