#include "http/HttpServer.h"
#include "InetAddress.h"
#include "TcpConnection.h"
#include "TcpServer.h"
#include "http/HttpRequestBuilder.h"

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

    server_.onReciveMessageCallback = [this](const TcpConnectionPtr& conn, Buffer* buffer) {
        onReciveHttpMessage(conn, buffer);
    };
}

void HttpServer::onEstablishNewConnection(const TcpConnectionPtr& conn, InetAddress addr)
{
    conn->set_context(HttpRequestBuilder());
}

void HttpServer::onReciveHttpMessage(const TcpConnectionPtr& conn, Buffer* buffer) const
{
    auto& parser = std::any_cast<HttpRequestBuilder&>(conn->context());

    parser.parase(buffer);

    if (parser.hasError())
        conn->shutdown();
    else if (parser.ready())
        onHandleHttpRequest_(parser.request(), conn);
    else
        TRA << conn->fd() << " receive unready http message";
}
