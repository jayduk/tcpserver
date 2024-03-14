#include "http/HttpContext.h"
#include "common/ByteBuffer.h"
#include "http-parser/http_parser.h"
#include <string>
#include <sys/types.h>

HttpContext::HttpContext()
{
    http_parser_init(&parser_, HTTP_REQUEST);
    parser_.data = this;

    settings_.on_message_begin = [](http_parser* parser) -> int {
        auto* context     = static_cast<HttpContext*>(parser->data);
        context->request_ = std::make_shared<HttpRequest>();
        return 0;
    };

    settings_.on_url = [](http_parser* parser, const char* at, size_t length) -> int {
        auto* context = static_cast<HttpContext*>(parser->data);
        auto  request = context->request_;

        request->uri_.append(at, length);
        return 0;
    };

    settings_.on_header_field = [](http_parser* parser, const char* at, size_t length) -> int {
        auto* context = static_cast<HttpContext*>(parser->data);
        auto  request = context->request_;

        if (context->is_parsing_field) {
            context->header_field_.append(at, length);
        } else {
            context->header_field_ = std::string(at, length);
        }

        context->is_parsing_field = true;
        return 0;
    };

    settings_.on_header_value = [](http_parser* parser, const char* at, size_t length) -> int {
        auto* context = static_cast<HttpContext*>(parser->data);
        auto  request = context->request_;

        if (context->is_parsing_field) {
            request->headers_[context->header_field_] = std::string(at, length);
        } else {
            request->headers_[context->header_field_].append(at, length);
        }

        context->is_parsing_field = false;
        return 0;
    };

    settings_.on_headers_complete = [](http_parser* parser) -> int {
        auto* context = static_cast<HttpContext*>(parser->data);
        auto  request = context->request_;

        request->set_method(static_cast<HttpMethod>(parser->method));
        request->set_version(static_cast<HttpVersion>(parser->http_major * 10 + parser->http_minor));

        context->handle_request();

        return 0;
    };

    settings_.on_body = [](http_parser* parser, const char* at, size_t length) -> int {
        auto* context = static_cast<HttpContext*>(parser->data);
        auto  request = context->request_;

        request->body_buffer_.append(at, length);

        return 0;
    };

    settings_.on_message_complete = [](http_parser* parser) -> int {
        auto* context = static_cast<HttpContext*>(parser->data);
        auto& request = context->request_;

        request.reset();
        return 0;
    };
}

bool HttpContext::handle(ByteBuffer<>* buffer)
{
    auto clips = buffer->clips();
    for (auto clip : clips) {
        int parsed = (int)http_parser_execute(&parser_, &settings_, clip.first, clip.second);
        if (parsed != clip.second) {
            return false;
        }

        if (parser_.upgrade) {
            DBG << "Not support upgrade protocol";
            return false;
        }

        if (parser_.http_errno != 0) {
            DBG << "http parser error: " << http_errno_name(HTTP_PARSER_ERRNO(&parser_));
            return false;
        }
    }
    return true;
}

void HttpContext::handle_request()
{
    if (requests_.empty()) {
        requests_.push_back(request_);

        thread_pool_->commit([this]() {
            exec_on_thread();
        });
    } else {
        requests_.push_back(request_);
    }
}

void HttpContext::exec_on_thread()
{
    while (!requests_.empty()) {
        auto request = requests_.front();
        requests_.pop_back();

        // handle request
    }

    // handle request
}