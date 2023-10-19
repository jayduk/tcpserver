#include "TcpConnection.h"
#include "common/Buffer.h"

#include <map>
#include <string>

class HttpRequestBuilder;

class HttpRequest
{
    friend class HttpRequestBuilder;

private:
    std::string method_;
    std::string uri_;
    std::string version_;

    std::map<std::string, std::string> headers_;
    Buffer body_buffer_;

public:
    HttpRequest() = default;
    ~HttpRequest() = default;

public:
    void set_header(const std::string& key, const std::string& value);
    [[nodiscard]] std::string header(const std::string& key) const;

    [[nodiscard]] std::string uri() const;
    [[nodiscard]] std::string method() const;
    [[nodiscard]] Buffer* inputBuffer();
};
