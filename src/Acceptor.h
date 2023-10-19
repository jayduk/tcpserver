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
    bool edge_mode_;

    std::unique_ptr<Channel> acceptor_channel_;

    std::function<void(int fd, InetAddress addr)> onNewConnection;

public:
    Acceptor(ReactorEventLoop* loop, const InetAddress& addr, bool edge_mode = true);
    ~Acceptor() = default;

public:
    void registryNewConnectionCallback(std::function<void(int fd, InetAddress addr)>&& call)
    {
        onNewConnection = std::move(call);
    };

private:
    void onReadable();
};

#endif  // ACCEPTOR_H_
