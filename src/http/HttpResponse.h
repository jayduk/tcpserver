#ifndef HTTP_HTTPRESPONSE_H_
#define HTTP_HTTPRESPONSE_H_

#include "common/ByteBuffer.h"
#include "http/HttpStream.h"
#include "http/common.h"
#include "tcp/TcpConnection.h"
#include <map>
#include <memory>

class HttpResponse
{
    friend class HttpContext;

private:
    TcpConnectionPtr conn_;

    HttpVersion version_;
    int         status_code_;
    std::string reason_phrase_;

    std::map<std::string, std::string> headers_;

    HttpStream body_buffer_;

    bool is_chunked_;
    bool is_header_sent_;

public:
    HttpResponse();
    ~HttpResponse() = default;

public:
    void        set_status_code(int code, const std::string& phrase);
    void        set_header(const std::string& key, const std::string& value);
    HttpStream* output_stream();

    void flush();
    void to_bytebuffer(ByteBuffer<>* buffer, bool end = false);

private:
    void set_chunked();
};

using HttpResponsePtr = std::shared_ptr<HttpResponse>;

#endif  // HTTP_HTTPRESPONSE_H_