#include "http/HttpServer.h"
#include "common/ByteBuffer.h"
#include "http/HttpContext.h"
#include "tcp/InetAddress.h"
#include "tcp/TcpConnection.h"
#include "util/ThreadPool.h"

#include <any>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <utility>

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

void HttpServer::setIoLoopCount(int num)
{
    server_.setThreadNum(num);
};

void HttpServer::setMapping(HttpRouteMapping* mapping)
{
    mapping_ = mapping;
}

void HttpServer::onEstablishConnection(const TcpConnectionPtr& conn, InetAddress addr)
{
    conn->set_context(new HttpContext(&pool_, conn, [this](auto&& req, auto&& resp) {
        onHandleRequest(std::forward<decltype(req)>(req), std::forward<decltype(resp)>(resp));
    }));
    INF << "HttpServer::onEstablishConnection: " << addr.ipString() << ":" << addr.port();
}

void HttpServer::onReceiveHttpMessage(const TcpConnectionPtr& conn, ByteBuffer<>* buffer)
{
    auto context = static_cast<HttpContext*>(conn->context());
    if (!context->handle(buffer)) {
        conn->shutdown();
    }
}

void HttpServer::onHandleRequest(const std::shared_ptr<HttpRequest>& request, const std::shared_ptr<HttpResponse>& response)
{
    auto fn = mapping_->find(request->method(), request->uri(), request->path_params());
    if (fn) {
        fn(request, response);
    } else {
        response->set_status_code(404, "Not Found");
    }
}