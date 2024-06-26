#ifndef HTTP_HTTPSERVER_H_
#define HTTP_HTTPSERVER_H_

#include "common/ByteBuffer.h"
#include "common/noncopyable.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "http/HttpRouteMapping.h"
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
    TcpServer         server_;
    ThreadPool        pool_;
    HttpRouteMapping* mapping_;

public:
    HttpServer(ReactorEventLoop* loop, uint16_t port);

    void setIoLoopCount(int num);
    void setMapping(HttpRouteMapping* mapping);

private:
    void        onEstablishConnection(const TcpConnectionPtr& conn, InetAddress addr);
    static void onReceiveHttpMessage(const TcpConnectionPtr& conn, ByteBuffer<>* buffer);
    void        onHandleRequest(const std::shared_ptr<HttpRequest>& request, const std::shared_ptr<HttpResponse>& response);
};

#endif  // HTTP_HTTPSERVER_H_