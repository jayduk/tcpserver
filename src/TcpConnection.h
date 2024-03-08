#ifndef TCPCONNECTION_H_
#define TCPCONNECTION_H_

#include "Channel.h"
#include "ReactorEventLoop.h"
#include "common/ByteBuffer.h"
#include "common/noncopyable.h"
#include <functional>
#include <memory>
#include <string>

#include <any>

class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
private:
    int      fd_;
    bool     running;
    std::any context_;

    ReactorEventLoop*        loop_;
    std::unique_ptr<Channel> channel_;

    ByteBuffer<> read_buffer_;
    ByteBuffer<> write_buffer_;

public:
    std::function<void(TcpConnectionPtr, ByteBuffer<>*)> onReciveMessageCallback;
    std::function<void(int)>                             onConnectionCloseCallback;

public:
    TcpConnection(ReactorEventLoop* loop, int fd_, bool edge_mode = true);
    ~TcpConnection() override;

    void init();

    void send(const std::string& msg);
    void shutdown();

    std::any& context();

    void set_context(std::any&& context);

    int fd() const;

private:
    void handleRead();
    void handleWrite();
    void handleClose();

private:
    void sendInloop(const std::string& msg);
};

#endif  // TCPCONNECTION_H_