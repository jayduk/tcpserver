#ifndef HTTP_HTTPREQUESTBUILDER_H_
#define HTTP_HTTPREQUESTBUILDER_H_

#include "common/Buffer.h"
#include "common/noncopyable.h"
#include "http/HttpRequest.h"
#include <cstddef>
#include <iostream>

constexpr int HTTP_PROTOCOL_NOT_HTTP1_1 = 1;
constexpr int HTTP_HEADER_UNEXPECTED_HEADER = 2;
constexpr int HTTP_CONTENT_LENGTH_LT_ZERO = 3;

class HttpRequestBuilder
{
public:
private:
    enum class BuildState
    {
        kBuildStartLine,
        kBuildHeader,
        kBuildBody,
        kBuildOk
    };

private:
    HttpRequest parsing_request_;

    BuildState build_state_;
    int error_code_{};

public:
    HttpRequestBuilder();
    ~HttpRequestBuilder() = default;

    void parase(Buffer* buffer);
    [[nodiscard]] bool ready() const;
    [[nodiscard]] bool hasError() const;
    [[nodiscard]] int error_code() const;
    HttpRequest request();

private:
    void buildStartLine(Buffer* buffer);
    void bulidHeader(Buffer* buffer);
    void bulidEntityBody(Buffer* buffer);
    void buildBodyByLength(Buffer* buffer);
    void buildBodyByChunk(Buffer* buffer);

    void set_error(int error);
    bool errorExit(int error);
};

#endif  // HTTP_HTTPREQUESTBUILDER_H_