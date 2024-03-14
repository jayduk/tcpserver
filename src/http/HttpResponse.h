#ifndef HTTP_HTTPRESPONSE_H_
#define HTTP_HTTPRESPONSE_H_

#include "common/ByteBuffer.h"
#include "http/HttpStream.h"
#include "tcp/TcpConnection.h"
#include <map>

class HttpResponse
{
private:
    TcpConnectionPtr conn_;

    std::string method_;
    int         status_code_;
    std::string reason_phrase_;

    std::map<std::string, std::string> headers_;

    HttpStream body_buffer_;

public:
    HttpResponse();
    ~HttpResponse() = default;

    void flush();
    void to_bytebuffer(ByteBuffer<>* buffer);
};

#endif  // HTTP_HTTPRESPONSE_H_