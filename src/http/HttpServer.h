#ifndef HTTP_HTTPSERVER_H_
#define HTTP_HTTPSERVER_H_

#include "InetAddress.h"
#include "ReactorEventLoop.h"
#include "TcpConnection.h"
#include "TcpServer.h"
#include "common/ByteBuffer.h"
#include "common/noncopyable.h"

#include <cstdint>
#include <functional>
#include <map>
#include <memory>

class HttpServer : noncopyable
{
private:
    TcpServer server_;

public:
    // std::function<void(HttpRequest, TcpConnectionPtr)> onHandleHttpRequest_;

public:
    HttpServer(ReactorEventLoop* loop, uint16_t port);

private:
    static void onEstablishNewConnection(const TcpConnectionPtr& conn, InetAddress addr);
    void        onReciveHttpMessage(const TcpConnectionPtr& conn, ByteBuffer<>* buffer) const;
};

#endif  // HTTP_HTTPSERVER_H_