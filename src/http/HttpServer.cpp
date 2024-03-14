#include "http/HttpServer.h"
#include "common/ByteBuffer.h"
#include "http/HttpContext.h"
#include "tcp/InetAddress.h"
#include "tcp/TcpConnection.h"

#include <any>
#include <memory>

HttpServer::HttpServer(ReactorEventLoop* loop, uint16_t port)
  : server_(loop, port)
{
    server_.setThreadNum(1);

    server_.new_connection_cb = [this](const TcpConnectionPtr& conn, InetAddress addr) {
        onEstablishConnection(conn, addr);
    };

    server_.handle_message_cb = [this](const TcpConnectionPtr& conn, ByteBuffer<>* buffer) {
        onReceiveHttpMessage(conn, buffer);
    };
}

void HttpServer::onEstablishConnection(const TcpConnectionPtr& conn, InetAddress addr)
{
    conn->set_context(std::make_any<HttpContext>());

    auto& http_context = std::any_cast<HttpContext&>(conn->context());
    http_context.set_thread_pool(&pool_);
}

void HttpServer::onReceiveHttpMessage(const TcpConnectionPtr& conn, ByteBuffer<>* buffer) const
{
    auto& context = std::any_cast<HttpContext&>(conn->context());
    if (!context.handle(buffer)) {
        conn->shutdown();
    }
}