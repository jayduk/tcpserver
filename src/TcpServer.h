#ifndef TCPSERVER_H_
#define TCPSERVER_H_

#include "Acceptor.h"
#include "InetAddress.h"
#include "ReactorEventLoop.h"
#include "ReactorEventLoopPool.h"
#include "TcpConnection.h"
#include "common/Buffer.h"
#include "common/ByteBuffer.h"

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
    TcpServer(ReactorEventLoop* loop, uint16_t port, bool edge_mode);
    ~TcpServer() = default;

public:
    void setThreadNum(int num);

public:
    std::function<void(TcpConnectionPtr, InetAddress)>   onEstablishNewConnectionCallback;
    std::function<void(TcpConnectionPtr, ByteBuffer<>*)> onReciveMessageCallback;

private:
    void        onNewConnection(int fd, InetAddress addr);
    static void onReciveMessage(TcpConnection* conn, Buffer* buffer);
};

#endif  // TCPSERVER_H_