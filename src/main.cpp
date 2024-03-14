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

#include "http/HttpRequest.h"

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

int main()
{
    configLogSettings();

    char s[] = "GET /host HTTP/1.1\r\n\
Transfer-Encoding: chunked\r\n\
Host: www.baidu.com\r\n\
Connection: keep-alive\r\n\
Cache-Control: max-age=0\r\n\
sec-ch-ua: \" Not A;Brand\";v=\"99\", \"Chromium\";v=\"96\", \"Google Chrome\";v=\"96\"\r\n\
sec-ch-ua-mobile: ?0\r\n\
sec-ch-ua-platform: \" macOS \"\r\n\
Upgrade-Insecure-Requests: 1\r\n\
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/96.0.4664.110 Safari/537.36\r\n\
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n\
Sec-Fetch-Site: none\r\n\
Sec-Fetch-Mode: navigate\r\n\
Sec-Fetch-User: ?1\r\n\
Sec-Fetch-Dest: document\r\n\
Accept-Encoding: gzip, deflate, br\r\n\
Accept-Language: zh-CN,zh;q=0.9,en;q=0.8\r\n\
Cookie: BIDUPSID=8B0207CE0B6364E5934651E84F17999B; PSTM=1619707475; \r\n\r\n";

    char chunked_body[] = "4\r\nWiki\r\n5\r\npedia\r\n0\r\n\r\n\
GET /host HTTP/1.1\r\n\
Transfer-Encoding: chunked\r\n\
Host: www.baidu.com\r\n\
Connection: keep-alive\r\n";

    HttpRequest request;

    auto i = request.parser(s, strlen(s));
    WAR << i;

    i = request.parser(chunked_body, strlen(chunked_body));
    WAR << i;
}
