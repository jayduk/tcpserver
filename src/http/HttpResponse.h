#ifndef HTTP_HTTPRESPONSE_H_
#define HTTP_HTTPRESPONSE_H_

#include "TcpConnection.h"
#include <map>

class HttpResponse
{
private:
    TcpConnectionPtr conn_;

    std::string method_;
    int         status_code_;
    std::string reason_phrase_;

    std::map<std::string, std::string> headers_;
    std::string                        data_;

public:
    HttpResponse();
    ~HttpResponse() = default;
};

#endif  // HTTP_HTTPRESPONSE_H_