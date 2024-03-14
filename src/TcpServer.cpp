#include "TcpServer.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "ReactorEventLoopPool.h"
#include "TcpConnection.h"

#include "log/easylogging++.h"
#include <cstdint>
#include <functional>
#include <memory>

TcpServer::TcpServer(ReactorEventLoop* loop, uint16_t port, bool edge_mode)
  : loop_(loop)
  , acceptor_(std::make_unique<Acceptor>(loop, InetAddress(port), edge_mode))
  , loop_pool_(std::make_unique<ReactorEventLoopPool>())
{
    acceptor_->registryNewConnectionCallback([this](int fd, InetAddress addr) {
        onNewConnection(fd, addr);
    });

    loop_pool_->setThreadNum(1);
}

void TcpServer::setThreadNum(int num)
{
    loop_pool_->setThreadNum(num);
}

void TcpServer::onNewConnection(int fd, InetAddress addr)
{
    INF << fd << " connected in " << addr.ipString() << ": " << addr.port();

    auto io_loop = loop_pool_->selectLoop();

    auto conn = std::make_shared<TcpConnection>(io_loop, fd, true);

    tcp_connections_.insert({fd, conn});

    conn->onConnectionCloseCallback = [this](int fd) {
        tcp_connections_.erase(fd);
    };

    if (onReciveMessageCallback) {
        conn->onReciveMessageCallback = onReciveMessageCallback;
    }

    if (onEstablishNewConnectionCallback) {
        onEstablishNewConnectionCallback(conn, addr);
    }

    io_loop->runInLoop([conn] {
        conn->init();
    });
}
