#ifndef __HTTPCONTEXT_H__
#define __HTTPCONTEXT_H__

#include "TcpConnection.h"
#include "common/ByteBuffer.h"
#include "http-parser/http_parser.h"
#include "http/HttpRequest.h"
#include "http/common.h"
#include "thread/threadpool.h"
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "log/easylogging++.h"

class HttpContext
{
    std::weak_ptr<TcpConnection> conn_;
    ThreadPool*                  thread_pool_;

    std::shared_ptr<HttpRequest>              request_;
    std::vector<std::shared_ptr<HttpRequest>> requests_;

    http_parser          parser_{};
    http_parser_settings settings_{};

    bool        is_parsing_field{};
    std::string header_field_{false};

public:
    HttpContext();
    void set_thread_pool(ThreadPool* pool) {}

public:
    bool handle(ByteBuffer<>* buffer);

private:
    void handle_request();
    void exec_on_thread();
};

#endif  //_HttpContext_h_