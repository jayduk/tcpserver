#include "TcpConnection.h"
#include "common/ByteBuffer.h"
#include "http-parser/http_parser.h"
#include "http/HttpStream.h"
#include "http/common.h"

#include <cstddef>
#include <map>
#include <string>

constexpr int PARSE_URL          = 0;
constexpr int PARSE_HEADER_FIELD = 1;
constexpr int PARSE_HEADER_VALUE = 2;
constexpr int PARSE_BODY         = 3;

class HttpRequest
{
    friend class HttpContext;

private:
    HttpMethod                         method_{};
    std::string                        uri_{};
    HttpVersion                        version_{};
    std::map<std::string, std::string> headers_{};

    HttpStream body_buffer_;

    http_parser          parser_{};
    http_parser_settings settings_{};

    int         status{PARSE_URL};
    std::string header_field_{};

public:
    HttpRequest();
    ~HttpRequest() = default;

    size_t             parser(const char* data, size_t size);
    [[nodiscard]] bool header_complete() const;

public:
    [[nodiscard]] std::string header(const std::string& key) const;
    [[nodiscard]] std::string uri() const;
    [[nodiscard]] HttpMethod  method() const;
    [[nodiscard]] HttpStream* inputBuffer();

private:
    void set_method(HttpMethod method);
    void set_url(const std::string& url);
    void set_version(HttpVersion version);
    void set_header(const std::string& key, const std::string& value);
};
