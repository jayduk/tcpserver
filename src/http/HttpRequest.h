#ifndef HTTP_HTTPREQUEST_H_
#define HTTP_HTTPREQUEST_H_

#include "common/ByteBuffer.h"
#include "http-parser/http_parser.h"
#include "http/HttpStream.h"
#include "http/common.h"

#include <cstddef>
#include <map>
#include <memory>
#include <string>

class HttpRequest
{
    friend class HttpContext;

private:
    HttpMethod                         method_{};
    std::string                        uri_{};
    HttpVersion                        version_{};
    std::map<std::string, std::string> headers_{};

    std::map<std::string, std::string> path_params_;

    HttpStream body_buffer_;

public:
    HttpRequest() = default;
    ~HttpRequest()
    {
        INF << "HttpRequest dtor";
    };

public:
    [[nodiscard]] std::string header(const std::string& key) const;
    [[nodiscard]] std::string uri() const;
    [[nodiscard]] HttpMethod  method() const;
    [[nodiscard]] HttpStream* input_stream();

    std::map<std::string, std::string>* path_params();

    std::string path_param(const std::string& key) const;

private:
    void set_method(HttpMethod method);
    void set_version(HttpVersion version);
    void set_header(const std::string& key, const std::string& value);
};

using HttpRequestPtr = std::shared_ptr<HttpRequest>;

#endif  // HTTP_HTTPREQUEST_H_