#include "http/HttpRequest.h"
#include "http/common.h"
#include "log/easylogging++.h"
#include <cstddef>
#include <string>

HttpRequest::HttpRequest()
{
    http_parser_init(&parser_, HTTP_REQUEST);

    parser_.data = this;

    settings_.on_message_begin = [](http_parser* parser) -> int {
        DBG << "on_message_begin";
        return 0;
    };

    settings_.on_url = [](http_parser* parser, const char* at, size_t length) -> int {
        auto request = static_cast<HttpRequest*>(parser->data);
        request->uri_.append(at, length);
        request->status = PARSE_URL;
        return 0;
    };

    settings_.on_header_field = [](http_parser* parser, const char* at, size_t length) -> int {
        DBG << "on_header_field";
        auto request = static_cast<HttpRequest*>(parser->data);
        if (request->status == PARSE_URL || request->status == PARSE_HEADER_VALUE) {
            request->header_field_ = std::string(at, length);
        } else if (request->status == PARSE_HEADER_FIELD) {
            request->header_field_.append(at, length);
        } else {
            return -1;
        }

        request->status = PARSE_HEADER_FIELD;
        return 0;
    };

    settings_.on_header_value = [](http_parser* parser, const char* at, size_t length) -> int {
        DBG << "on_header_value";
        auto request = static_cast<HttpRequest*>(parser->data);
        if (request->status == PARSE_HEADER_FIELD) {
            request->headers_[request->header_field_] = std::string(at, length);
        } else if (request->status == PARSE_HEADER_VALUE) {
            request->headers_[request->header_field_].append(at, length);
        } else {
            return -1;
        }

        request->status = PARSE_HEADER_VALUE;
        return 0;
    };

    settings_.on_headers_complete = [](http_parser* parser) -> int {
        auto request    = static_cast<HttpRequest*>(parser->data);
        request->status = PARSE_BODY;
        DBG << "on_headers_complete";
        return 0;
    };

    settings_.on_body = [](http_parser* parser, const char* at, size_t length) -> int {
        auto request = static_cast<HttpRequest*>(parser->data);
        request->body_buffer_.append(at, length);
        return 0;
    };

    settings_.on_message_complete = [](http_parser* parser) -> int {
        DBG << "on_message_complete";
        return 0;
    };
}

size_t HttpRequest::parser(const char* data, size_t size)
{
    return http_parser_execute(&parser_, &settings_, data, size);
}

bool HttpRequest::header_complete() const
{
    return status >= PARSE_BODY;
}

void HttpRequest::set_header(const std::string& key, const std::string& value)
{
    headers_[key] = value;
}

std::string HttpRequest::header(const std::string& key) const
{
    auto pheader = headers_.find(key);
    if (pheader != headers_.end()) {
        return pheader->second;
    }
    return "";
}

std::string HttpRequest::uri() const
{
    return uri_;
}
HttpMethod HttpRequest::method() const
{
    return method_;
}
HttpStream* HttpRequest::inputBuffer()
{
    return &body_buffer_;
}
