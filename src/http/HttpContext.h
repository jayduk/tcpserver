#ifndef __HTTPCONTEXT_H__
#define __HTTPCONTEXT_H__

#include "TcpConnection.h"
#include <map>
#include <memory>
#include <string>

#include "http/IHttpParser.h"
#include "http/common.h"

#include "log/easylogging++.h"

class HttpContext
{
    std::weak_ptr<TcpConnection> conn_;
    std::shared_ptr<IHttpParser> parser_;

    HttpMethod                         method_;
    std::string                        uri_;
    HttpVersion                        version_;
    std::map<std::string, std::string> headers_;

    int                                status_;
    std::string                        reason_phrase_;
    std::map<std::string, std::string> response_header_;
    // char*                              data_;

public:
    HttpContext()
    {
        INF << "construct of HttpContext";
    }
};

#endif  //_HttpContext_h_