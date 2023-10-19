#include "Acceptor.h"
#include "InetAddress.h"
#include "ReactorEventLoop.h"
#include "TcpConnection.h"
#include "TcpServer.h"
#include "http/HttpServer.h"
#include "thread/threadpool.h"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

#include "log/easylogging++.h"

#define PORT 9002

INITIALIZE_EASYLOGGINGPP

void configLogSettings()
{
    // Load configuration from file
    el::Configurations conf("config/log.config");
    // Reconfigure single logger
    el::Loggers::reconfigureLogger("default", conf);
    // Actually reconfigure all loggers instead
    el::Loggers::reconfigureAllLoggers(conf);

    el::Loggers::addFlag(el::LoggingFlag::StrictLogFileSizeCheck);
}

void onReciveMessage(TcpConnection* conn, Buffer buffer)
{
    auto msg = buffer.retrieveAllAsString();
    conn->send(msg);
}

int main()
{
    configLogSettings();

    // ReactorEventLoop loop;
    // TcpServer server(&loop, 9002, true);

    // server.onReciveMessageCallback = [](const TcpConnectionPtr& conn, Buffer* buffer) {
    //     conn->send(buffer->retrieveAllAsString());
    // };

    // loop.loop();
    ReactorEventLoop loop;
    HttpServer server(&loop, 9005);

    server.onHandleHttpRequest_ = [](HttpRequest r, const TcpConnectionPtr& c) {
        INF << r.method();
        INF << r.uri();
        INF << r.header("Content-Length");
        INF << r.inputBuffer();
        c->send("HELLO");
    };

    loop.loop();
}
