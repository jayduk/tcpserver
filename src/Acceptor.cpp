#include "Acceptor.h"
#include "InetAddress.h"
#include "Poller.h"
#include "ReactorEventLoop.h"
#include "sock.h"
#include <cerrno>
#include <cstdio>
#include <functional>
#include <memory>
#include <sys/socket.h>

#include "log/easylogging++.h"

Acceptor::Acceptor(ReactorEventLoop* loop, const InetAddress& addr, bool edge_mode)
  : loop_(loop)
  , listen_fd_(Socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0))
  , edge_mode_(edge_mode)
  , acceptor_channel_(std::make_unique<Channel>(loop, listen_fd_))
{
    Bind(listen_fd_, addr.addr(), addr.len());
    Listen(listen_fd_, 8000);

    INF << listen_fd_ << " Acceptor channel in " << addr.ipString() << ":" << addr.port();

    acceptor_channel_->enableReading();

    acceptor_channel_->readableCallback_ = [this] {
        onReadable();
    };
}

void Acceptor::onReadable()
{
    InetAddress client_addr;
    do
    {
        int accept_fd = accept(listen_fd_, client_addr.addr(), &client_addr.len());

        if (accept_fd == -1)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
                break;

            else if (errno == EINTR)
                continue;
            else
            {
                printf("%d \n", errno);
                break;
            }
        }

        SetNonBlockingSocket(accept_fd);

        if (onNewConnection)
            onNewConnection(accept_fd, client_addr);

    } while (edge_mode_);
}