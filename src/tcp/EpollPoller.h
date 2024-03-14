#ifndef EPOLLPOLLER_H_
#define EPOLLPOLLER_H_

#include "Poller.h"
#include "tcp/Channel.h"
#include <sys/epoll.h>
#include <vector>

class EpollPoller : public Poller
{
private:
    int epoll_fd_;

    std::vector<epoll_event> events_;

public:
    explicit EpollPoller(int poller_size = 200);
    ~EpollPoller() override;

    void wait(int wait_millis, std::vector<Channel*>& activeChannel) override;
    void updateChannel(Channel* channel) override;
};

#endif  // EPOLLPOLLER_H_