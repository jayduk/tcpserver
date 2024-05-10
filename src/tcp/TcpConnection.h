#ifndef TCPCONNECTION_H_
#define TCPCONNECTION_H_

#include "ReactorEventLoop.h"
#include "common/ByteBuffer.h"
#include "common/noncopyable.h"
#include "tcp/Channel.h"
#include <functional>
#include <memory>
#include <string>

#include <any>

class TcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
private:
    int   fd_;
    bool  running_;
    void* context_;

    ReactorEventLoop*        loop_;
    std::unique_ptr<Channel> channel_;

    ByteBuffer<> read_buffer_;
    ByteBuffer<> write_buffer_;

public:
    std::function<void(TcpConnectionPtr, ByteBuffer<>*)> receive_bytes_cb;
    std::function<void(int)>                             close_tcp_cb;

public:
    TcpConnection(ReactorEventLoop* loop, int fd_);
    ~TcpConnection() override;

    void init();

    void send(const std::string& msg);
    void send(ByteBuffer<>* buffer);
    void send(std::shared_ptr<ByteBuffer<>> buffer);
    void shutdown();

    void* context();
    void  set_context(void* context);

    int fd() const;

private:
    void handleRead();
    void handleWrite();
    void handleClose();

private:
    void sendInLoop(const std::string& msg);
    void sendInLoop(ByteBuffer<>* buffer);
    void sendInLoop(std::shared_ptr<ByteBuffer<>> buffer);
};

#endif  // TCPCONNECTION_H_