#include "http/HttpServer.h"
#include "InetAddress.h"
#include "TcpConnection.h"
#include "TcpServer.h"
#include "common/ByteBuffer.h"
#include "http/HttpContext.h"

#include <any>
#include <cstddef>
#include <memory>
#include <sys/types.h>
#include <utility>

HttpServer::HttpServer(ReactorEventLoop* loop, uint16_t port)
  : server_(loop, port, true)
{
    server_.setThreadNum(1);

    server_.onEstablishNewConnectionCallback = [this](const TcpConnectionPtr& conn, InetAddress addr) {
        onEstablishNewConnection(conn, addr);
    };

    server_.onReciveMessageCallback = [this](const TcpConnectionPtr& conn, ByteBuffer<>* buffer) {
        onReciveHttpMessage(conn, buffer);
    };
}

void HttpServer::onEstablishNewConnection(const TcpConnectionPtr& conn, InetAddress addr)
{
    conn->set_context(std::make_any<HttpContext>());
    auto& http_context = std::any_cast<HttpContext&>(conn->context());
    http_context.set_thread_pool(&pool_);
}

void HttpServer::onReciveHttpMessage(const TcpConnectionPtr& conn, ByteBuffer<>* buffer) const
{
    auto& context = std::any_cast<HttpContext&>(conn->context());
    if (!context.handle(buffer)) {
        conn->shutdown();
    }
}