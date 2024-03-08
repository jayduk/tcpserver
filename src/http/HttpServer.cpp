#include "http/HttpServer.h"
#include "InetAddress.h"
#include "TcpConnection.h"
#include "TcpServer.h"
#include "common/ByteBuffer.h"
#include "http/HttpContext.h"

#include <any>
#include <cstddef>
#include <memory>
#include <utility>

HttpServer::HttpServer(ReactorEventLoop* loop, uint16_t port)
  : server_(loop, port, true)
{
    server_.setThreadNum(500);

    server_.onEstablishNewConnectionCallback = [](const TcpConnectionPtr& conn, InetAddress addr) {
        onEstablishNewConnection(conn, addr);
    };

    server_.onReciveMessageCallback = [this](const TcpConnectionPtr& conn, ByteBuffer<>* buffer) {
        onReciveHttpMessage(conn, buffer);
    };
}

void HttpServer::onEstablishNewConnection(const TcpConnectionPtr& conn, InetAddress addr)
{
    conn->set_context(std::make_any<HttpContext>());
}

void HttpServer::onReciveHttpMessage(const TcpConnectionPtr& conn, ByteBuffer<>* buffer) const
{
    auto& parser = std::any_cast<HttpContext&>(conn->context());

    // int parsed = parser.parase(buffer);

    // if (parser.hasError())
    //     conn->shutdown();
    // else if (parser.ready())
    //     onHandleHttpRequest_(parser.request(), conn);
    // else
    //     TRA << conn->fd() << " receive unready http message";
}
