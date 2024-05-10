#ifndef __HTTPCONTEXT_H__
#define __HTTPCONTEXT_H__

#include "common/ByteBuffer.h"
#include "common/noncopyable.h"
#include "http-parser/http_parser.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "http/HttpRouteMapping.h"
#include "http/common.h"
#include "tcp/TcpConnection.h"
#include "util/ThreadPool.h"
#include <cstddef>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <string>
#include <vector>

#include "log/easylogging++.h"

class HttpContext
{
    std::weak_ptr<TcpConnection> conn_;
    ThreadPool*                  thread_pool_{nullptr};
    HttpHandleFn                 handle_fn_{nullptr};

    std::shared_ptr<HttpRequest>             request_;
    std::queue<std::shared_ptr<HttpRequest>> requests_;

    std::mutex requests_mt_;

    http_parser          parser_{};
    http_parser_settings settings_{};

    bool        is_parsing_field{};
    std::string header_field_{false};

public:
    HttpContext(ThreadPool* pool, const std::shared_ptr<TcpConnection>& conn, HttpHandleFn fn);
    ~HttpContext();

public:
    bool handle(ByteBuffer<>* buffer);

private:
    void handle_request();
    void exec_on_thread();

    std::shared_ptr<HttpResponse> build_response(const std::shared_ptr<HttpRequest>& request);
};

#endif  //_HttpContext_h_