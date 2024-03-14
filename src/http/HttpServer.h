#ifndef HTTP_HTTPSERVER_H_
#define HTTP_HTTPSERVER_H_

#include "common/ByteBuffer.h"
#include "common/noncopyable.h"
#include "tcp/InetAddress.h"
#include "tcp/ReactorEventLoop.h"
#include "tcp/TcpConnection.h"
#include "tcp/TcpServer.h"
#include "util/ThreadPool.h"
#include <cstdint>
#include <functional>
#include <map>
#include <memory>

class HttpServer : noncopyable
{
private:
    TcpServer  server_;
    ThreadPool pool_;

public:
public:
    HttpServer(ReactorEventLoop* loop, uint16_t port);

private:
    void onEstablishConnection(const TcpConnectionPtr& conn, InetAddress addr);
    void onReceiveHttpMessage(const TcpConnectionPtr& conn, ByteBuffer<>* buffer) const;
};

#endif  // HTTP_HTTPSERVER_H_