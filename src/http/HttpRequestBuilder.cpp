#include "http/HttpRequestBuilder.h"
#include "common/Buffer.h"
#include "log/easylogging++.h"

#include <cstddef>
#include <cstdlib>
#include <string>
#include <sys/types.h>
#include <type_traits>

HttpRequestBuilder::HttpRequestBuilder()
  : build_state_(BuildState::kBuildStartLine)
{
}

void HttpRequestBuilder::parase(Buffer* buffer)
{
    if (build_state_ == BuildState::kBuildOk)
        return;

    if (build_state_ == BuildState::kBuildStartLine && !hasError())
    {
        buildStartLine(buffer);
    }

    if (build_state_ == BuildState::kBuildHeader && !hasError())
    {
        bulidHeader(buffer);
    }

    if (build_state_ == BuildState::kBuildBody && !hasError())
    {
        bulidEntityBody(buffer);
    }
}

bool HttpRequestBuilder::ready() const
{
    return build_state_ == BuildState::kBuildOk;
}

bool HttpRequestBuilder::hasError() const
{
    return error_code_ != 0;
}

int HttpRequestBuilder::error_code() const
{
    return error_code_;
}

HttpRequest HttpRequestBuilder::request()
{
    HttpRequest request;

    using std::swap;
    swap(request, parsing_request_);
    return request;
}

void HttpRequestBuilder::buildStartLine(Buffer* buffer)
{
    auto crlf = buffer->cacheFindFirst("\r\n");

    if (crlf == nullptr)
        return;

    char* space = buffer->findFirst(' ', buffer->peek(), crlf);
    if (space)
    {
        parsing_request_.method_ = std::string(buffer->peek(), space);
        buffer->retrieveUtil(space + 1);
    }

    space = buffer->findFirst(' ');
    if (space)
    {
        parsing_request_.uri_ = std::string(buffer->peek(), space);
        buffer->retrieveUtil(space + 1);
    }

    parsing_request_.version_ = std::string(buffer->peek(), crlf);
    buffer->retrieveUtil(crlf + 2);

    if (parsing_request_.version_ != "HTTP/1.1")
    {
        set_error(HTTP_PROTOCOL_NOT_HTTP1_1);
    }

    build_state_ = BuildState::kBuildHeader;
}

void HttpRequestBuilder::bulidHeader(Buffer* buffer)
{
    while (true)
    {
        char* crlf = buffer->cacheFindFirst("\r\n");

        if (crlf == nullptr)
            return;

        if (crlf == buffer->peek())
        {
            buffer->retrieve(2);
            break;
        }

        char* mid = buffer->findFirst(':', buffer->peek(), crlf);

        if (!mid)
        {
            set_error(HTTP_HEADER_UNEXPECTED_HEADER);
            return;
        }

        parsing_request_.set_header(std::string(buffer->peek(), mid), std::string(mid + 1, crlf));

        buffer->retrieveUtil(crlf + 2);
    }

    build_state_ = BuildState::kBuildBody;
}

void HttpRequestBuilder::bulidEntityBody(Buffer* buffer)
{
    if (parsing_request_.method() == "GET" ||
        parsing_request_.method() == "HEAD" ||
        parsing_request_.method() == "TRACE" ||
        parsing_request_.method() == "OPTIONS" ||
        parsing_request_.method() == "DELETE")
    {
        build_state_ = BuildState::kBuildOk;
        return;
    }

    if (parsing_request_.header("Transfer-Encoding") == "Chunk")
        buildBodyByChunk(buffer);
    else
        buildBodyByLength(buffer);
}

void HttpRequestBuilder::buildBodyByLength(Buffer* buffer)
{
    if (parsing_request_.header("Content-Length").empty())
    {
        build_state_ = BuildState::kBuildOk;
        return;
    }

    auto needed_length = stol(parsing_request_.header("Content-Length")) - (long)parsing_request_.body_buffer_.readableBytes();

    if (needed_length < 0)
        return set_error(HTTP_CONTENT_LENGTH_LT_ZERO);

    std::string temp;
    size_t size = buffer->tryReadString(needed_length - parsing_request_.body_buffer_.readableBytes(), &temp);
    parsing_request_.body_buffer_.append(temp);

    if (size == (size_t)needed_length)
    {
        build_state_ = BuildState::kBuildOk;
        return;
    }
}

void HttpRequestBuilder::buildBodyByChunk(Buffer* buffer)
{
}

void HttpRequestBuilder::set_error(int error_code)
{
    error_code_ = error_code;
}