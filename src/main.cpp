#include "Acceptor.h"
#include "InetAddress.h"
#include "ReactorEventLoop.h"
#include "TcpConnection.h"
#include "TcpServer.h"
#include "http/HttpServer.h"
#include "thread/threadpool.h"
#include <bits/types/struct_timeval.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/select.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "log/easylogging++.h"

#include "http-parser/http_parser.h"

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

int onMessageBegin(http_parser* pParser)
{
    printf("@onMessageBegin call \n");
    return 0;
}

int onHeaderComplete(http_parser* pParser)
{
    printf("@onHeaderComplete call \n");
    return 0;
}
int onMessageComplete(http_parser* pParser)
{
    std::vector<int> v;
    printf("@onMessageComplete call \n");
    return 0;
}

int onURL(http_parser* pParser, const char* at, size_t length)
{
    printf("@onURL call, length:[%ld] \n", length);

    printf("@onURL url:[%s] \n", std::string(at, at + length).c_str());
    return 0;
}
int onStatus(http_parser* pParser, const char* at, size_t length)
{
    printf("@onStatus call, length:[%d] \n", length);

    printf("@onStatus status:[%s] \n", std::string(at, at + length).c_str());
    return 0;
}
int onHeaderField(http_parser* pParser, const char* at, size_t length)
{
    printf("@onHeaderField call, length:[%d] \n", length);

    printf("@onHeaderField field:[%s] \n", std::string(at, at + length).c_str());
    return 0;
}
int onHeaderValue(http_parser* pParser, const char* at, size_t length)
{
    printf("@onHeaderValue call, length:[%d] \n", length);
    std::string strValue(at, length);
    printf("@onHeaderValue value:[%s] \n", strValue.c_str());

    return 0;
}
int onBody(http_parser* pParser, const char* at, size_t length)
{
    printf("@onBody call, length:[%d] \n", length);
    printf("@onBody recv:[%s] \n", std::string(at, length).c_str());
    return 0;
}

int main()
{
    configLogSettings();

    //     http_parser          parser;
    //     http_parser_settings settings;
    //     http_parser_init(&parser, HTTP_REQUEST);
    //     http_parser_settings_init(&settings);

    //     settings.on_message_begin    = onMessageBegin;
    //     settings.on_headers_complete = onHeaderComplete;
    //     settings.on_message_complete = onMessageComplete;
    //     settings.on_url              = onURL;
    //     settings.on_status           = onStatus;
    //     settings.on_header_field     = onHeaderField;
    //     settings.on_header_value     = onHeaderValue;
    //     settings.on_body             = onBody;

    //     char s[] = "POST /DEMOWebServices2.8/Service.asmx/CancelOrder HTTP/1.1\r\n\
// Host: api.efxnow.com\r\n\
// Content-Type: application/x-www-form-urlencoded\r\n\
// Content-Length: 53\r\n\
// \r\n\
// UserID=string&PWD=string&OrderConfirmation=string    POST /DEMOWebServices2.8/Ser4444444444444";

    //     char* data = s;
    //     for (int i = 0; i < 6; ++i)
    //     {
    //         auto nParseBytes = http_parser_execute(&parser, &settings, data, 42);
    //         INF << "n parse = " << nParseBytes;
    //         data += 42;
    //     }

    // ReactorEventLoop loop;
    // TcpServer server(&loop, 9002, true);

    // server.onReciveMessageCallback = [](const TcpConnectionPtr& conn, Buffer* buffer) {
    //     conn->send(buffer->retrieveAllAsString());
    // };

    // loop.loop();
    ReactorEventLoop loop;

    TcpServer server(&loop, 45678, true);
    server.setThreadNum(1);
    server.onReciveMessageCallback = [](TcpConnectionPtr conn, ByteBuffer<>* buffer) {
        auto s = buffer->retrieve_as_string();
        conn->send(s);
    };

    // loop.runEvery(
    //     1000,
    //     [] {
    //         INF << "HELLO";
    //     },
    //     5000);
    loop.start();
    loop.loop();
}
