#ifndef TCPCONNECTION_H_
#define TCPCONNECTION_H_

#include "Channel.h"
#include "ReactorEventLoop.h"
#include "common/Buffer.h"
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

    Buffer read_buffer_;
    Buffer write_buffer_;

public:
    std::function<void(TcpConnectionPtr, Buffer*)> onReciveMessageCallback;
    std::function<void(int)>                       onConnectionCloseCallback;

public:
    TcpConnection(ReactorEventLoop* loop, int fd_, bool edge_mode = true);
    ~TcpConnection() override;

    void init();

    void send(const std::string& msg);
    void shutdown();

    std::any& context();

    void set_context(const std::any& context);
    void set_context(std::any&& context);

    int fd() const;

private:
    void handleRead();
    void handleWrite();
    void handleClose();

private:
    bool readAllMessage();
    void sendInloop(const std::string& msg);
};

#endif  // TCPCONNECTION_H_