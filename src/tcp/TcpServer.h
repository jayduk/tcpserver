#ifndef TCPSERVER_H_
#define TCPSERVER_H_

#include "ReactorEventLoop.h"
#include "ReactorEventLoopPool.h"
#include "TcpConnection.h"
#include "common/Buffer.h"
#include "common/ByteBuffer.h"
#include "tcp/Acceptor.h"
#include "tcp/InetAddress.h"

#include <cstdint>
#include <functional>
#include <memory>

class TcpServer
{
private:
    ReactorEventLoop*                     loop_;
    std::unique_ptr<Acceptor>             acceptor_;
    std::unique_ptr<ReactorEventLoopPool> loop_pool_;
    std::map<int, TcpConnectionPtr>       tcp_connections_;

public:
    TcpServer(ReactorEventLoop* loop, uint16_t port);
    ~TcpServer() = default;

public:
    void setThreadNum(int num);

public:
    std::function<void(TcpConnectionPtr, InetAddress)>   new_connection_cb;
    std::function<void(TcpConnectionPtr, ByteBuffer<>*)> handle_message_cb;

private:
    void onAcceptConnection(int fd, InetAddress addr);
};

#endif  // TCPSERVER_H_