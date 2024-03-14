#include "common/ByteBuffer.h"
#include "http-parser/http_parser.h"
#include "http/HttpStream.h"
#include "http/common.h"

#include <cstddef>
#include <map>
#include <string>

class HttpRequest
{
    friend class HttpContext;

private:
    HttpMethod                         method_{};
    std::string                        uri_{};
    HttpVersion                        version_{};
    std::map<std::string, std::string> headers_{};

    HttpStream body_buffer_;

public:
    HttpRequest()  = default;
    ~HttpRequest() = default;

public:
    [[nodiscard]] std::string header(const std::string& key) const;
    [[nodiscard]] std::string uri() const;
    [[nodiscard]] HttpMethod  method() const;
    [[nodiscard]] HttpStream* inputBuffer();

private:
    void set_method(HttpMethod method);
    void set_version(HttpVersion version);
    void set_header(const std::string& key, const std::string& value);
};
