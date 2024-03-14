#ifndef ACCEPTOR_H_
#define ACCEPTOR_H_

#include "Channel.h"
#include "InetAddress.h"
#include "Poller.h"
#include "ReactorEventLoop.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <utility>

class Acceptor
{
private:
    ReactorEventLoop* loop_;

    int listen_fd_;

    std::unique_ptr<Channel> acceptor_channel_;

public:
    Acceptor(ReactorEventLoop* loop, const InetAddress& addr);
    ~Acceptor() = default;

    std::function<void(int fd, InetAddress addr)> accept_connection_cb;

private:
    void onReadable();
};

#endif  // ACCEPTOR_H_
