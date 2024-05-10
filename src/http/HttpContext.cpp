#include "http/HttpContext.h"
#include "common/ByteBuffer.h"
#include "http-parser/http_parser.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "http/HttpRouteMapping.h"
#include <memory>
#include <mutex>
#include <string>
#include <sys/types.h>

HttpContext::HttpContext(ThreadPool* pool, const std::shared_ptr<TcpConnection>& conn, HttpHandleFn fn)
  : conn_(conn)
  , thread_pool_(pool)
  , handle_fn_(fn)
{
    http_parser_init(&parser_, HTTP_REQUEST);
    http_parser_settings_init(&settings_);

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

        INF << "headers complete";
        return 0;
    };

    settings_.on_body = [](http_parser* parser, const char* at, size_t length) -> int {
        auto* context = static_cast<HttpContext*>(parser->data);
        auto  request = context->request_;

        request->body_buffer_.write(at, length);

        INF << "body: ";

        return 0;
    };

    settings_.on_message_complete = [](http_parser* parser) -> int {
        auto* context = static_cast<HttpContext*>(parser->data);
        context->request_.reset();
        INF << "message complete";
        return 0;
    };
}

HttpContext::~HttpContext()
{
    INF << "HttpContext dtor";
}

bool HttpContext::handle(ByteBuffer<>* buffer)
{
    auto clips = buffer->clips();
    for (auto clip : clips) {
        INF << "parse clip " << clip.second << " bytes :" << std::string(clip.first, clip.second);
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
    buffer->retrieve_all();
    return true;
}

void HttpContext::handle_request()
{
    std::unique_lock<std::mutex> lock(requests_mt_);

    if (requests_.empty()) {
        thread_pool_->commit([this]() {
            exec_on_thread();
        });
    }

    requests_.push(request_);
}

void HttpContext::exec_on_thread()
{
    std::unique_lock<std::mutex> lock(requests_mt_);

    while (!requests_.empty()) {
        std::shared_ptr<HttpRequest> request = requests_.front();
        requests_.pop();

        lock.unlock();

        // handle request
        std::shared_ptr<HttpResponse> response = build_response(request);

        if (handle_fn_) {
            handle_fn_(request, response);
        }

        auto buffer = std::make_shared<ByteBuffer<>>();

        response->to_bytebuffer(buffer.get(), true);
        response->conn_->send(buffer);

        lock.lock();
    }
}

std::shared_ptr<HttpResponse> HttpContext::build_response(const std::shared_ptr<HttpRequest>& request)
{
    auto resp = std::make_shared<HttpResponse>();

    resp->conn_ = conn_.lock();

    resp->version_ = request->version_;

    resp->headers_["Connection"]   = request->header("Connection");
    resp->headers_["Content-Type"] = "text/html";

    return resp;
}