#ifndef POLLER_H_
#define POLLER_H_

#include "common/noncopyable.h"
#include <vector>

class Channel;

class Poller : noncopyable
{
public:
    virtual void wait(int wait_millis, std::vector<Channel*>& activeChannel) = 0;
    virtual void updateChannel(Channel* channel) = 0;
};

#endif  // POLLER_H_