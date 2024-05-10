#include "TcpServer.h"
#include "ReactorEventLoopPool.h"
#include "TcpConnection.h"
#include "tcp/Acceptor.h"
#include "tcp/InetAddress.h"

#include "log/easylogging++.h"
#include <cstdint>
#include <memory>

TcpServer::TcpServer(ReactorEventLoop* loop, uint16_t port)
  : loop_(loop)
  , acceptor_(std::make_unique<Acceptor>(loop, InetAddress(port)))
  , loop_pool_(std::make_unique<ReactorEventLoopPool>())
{
    acceptor_->accept_connection_cb = [this](int fd, InetAddress addr) {
        onAcceptConnection(fd, addr);
    };

    loop_pool_->setThreadNum(1);
}

void TcpServer::setThreadNum(int num)
{
    loop_pool_->setThreadNum(num);
}

void TcpServer::onAcceptConnection(int fd, InetAddress addr)
{
    INF << fd << " connected in " << addr.ipString() << ": " << addr.port();

    auto io_loop = loop_pool_->selectLoop();

    auto conn = std::make_shared<TcpConnection>(io_loop, fd);

    tcp_connections_.insert({fd, conn});

    conn->close_tcp_cb = [this](int fd) {
        tcp_connections_.erase(fd);
    };

    if (handle_message_cb) {
        conn->receive_bytes_cb = handle_message_cb;
    }

    if (new_connection_cb) {
        new_connection_cb(conn, addr);
    }

    io_loop->runInLoop([conn] {
        conn->init();
    });

    INF << "TcpServer::onAcceptConnection end";
}
