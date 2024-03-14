
#include "http/HttpRequest.h"
#include "log/easylogging++.h"
#include "tcp/TcpServer.h"
#include "util/ThreadPool.h"
#include <vector>

INITIALIZE_EASYLOGGINGPP

void configLogSettings()
{
    el::Helpers::setThreadName("main  ");
    // Load configuration from file
    el::Configurations conf("config/log.config");
    // Reconfigure single logger
    el::Loggers::reconfigureLogger("default", conf);
    // Actually reconfigure all loggers instead
    el::Loggers::reconfigureAllLoggers(conf);

    el::Loggers::addFlag(el::LoggingFlag::StrictLogFileSizeCheck);
}

int main()
{
    configLogSettings();

    ReactorEventLoop loop;
    TcpServer        server(&loop, 8859);

    server.handle_message_cb = [](const TcpConnectionPtr& conn, ByteBuffer<>* buffer) {
        conn->send(buffer->retrieve_as_string());
    };

    loop.loop();
}
