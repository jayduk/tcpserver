
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "http/HttpRouteMapping.h"
#include "http/HttpServer.h"
#include "http/common.h"
#include "log/easylogging++.h"
#include "tcp/TcpServer.h"
#include "util/ThreadPool.h"
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

INITIALIZE_EASYLOGGINGPP

void configLogSettings()
{
    el::Helpers::setThreadName("main  ");
    // Load configuration from file
    el::Configurations conf("config/log.config");
    // Reconfigure single logger
    el::Loggers::reconfigureLogger("default", conf);
    // Actually reconfigure all loggers instead
    el::Loggers::reconfigureAllLoggers(conf);

    el::Loggers::addFlag(el::LoggingFlag::StrictLogFileSizeCheck);
}

void initRouteMapping(HttpRouteMapping* mapping)
{
    mapping->add_default(HttpMethod::GET, [](std::shared_ptr<HttpRequest> request, std::shared_ptr<HttpResponse> response) {
        const static std::map<std::string, std::string> mime = {
            {".html", "text/html"},
            {".avi", "video/x-msvideo"},
            {".bmp", "image/bmp"},
            {".c", "text/plain"},
            {".doc", "application/msword"},
            {".gif", "image/gif"},
            {".gz", "application/x-gzip"},
            {".htm", "text/html"},
            {".ico", "image/x-icon"},
            {".jpg", "image/jpeg"},
            {".png", "image/png"},
            {".txt", "text/plain"},
            {".mp3", "audio/mp3"}};

        auto mime_type = [](const std::string& ext) -> std::string {
            auto it = mime.find(ext);
            if (it != mime.end()) {
                return it->second;
            }
            return "text/plain";
        };

        auto uri = request->uri();

        std::filesystem::path path = "static" + request->uri();

        if (!std::filesystem::exists(path)) {
            response->set_status_code(404, "Not Found");
            response->output_stream()->write("404 Not Found");
            return;
        }

        if (std::filesystem::is_directory(path)) {
            response->set_status_code(403, "Forbidden");
            response->output_stream()->write("403 Forbidden");
            return;
        }

        response->set_header("Content-Type", mime_type(path.extension().string()));

        response->set_status_code(200, "OK");

        std::ifstream file(path, std::ios::binary);
        if (file.is_open()) {
            char  buffer[65001];
            auto* output_stream = response->output_stream();

            while (!file.eof()) {
                file.read(buffer, sizeof(buffer));
                output_stream->write(buffer, file.gcount());
                response->flush();
            }

            file.close();
        }
    });

    mapping->add(HttpMethod::GET, "/", [](std::shared_ptr<HttpRequest> request, std::shared_ptr<HttpResponse> response) {
        response->set_status_code(301, "Moved Permanently");
        response->set_header("Location", "http://1.92.122.197/auth/oauth2/github?code=123");
    });

    mapping->add(HttpMethod::GET, "/auth/:id", [](std::shared_ptr<HttpRequest> request, std::shared_ptr<HttpResponse> response) {
        INF << "/auth/:id ----->" << request->uri();

        response->output_stream()->write("hello " + request->path_param("id"));
    });
    mapping->add(HttpMethod::GET, "/auth/:name/:id", [](std::shared_ptr<HttpRequest> request, std::shared_ptr<HttpResponse> response) {
        INF << "/auth/:id ----->" << request->uri();

        response->output_stream()->write("hello " + request->path_param("name"));
        response->output_stream()->write("\nhello " + request->path_param("id"));
    });

    INF << "initRouteMapping done";
}

int main()
{
    configLogSettings();

    ReactorEventLoop loop;

    HttpServer http_server(&loop, 40004);

    HttpRouteMapping route_mapping;
    initRouteMapping(&route_mapping);

    http_server.setMapping(&route_mapping);

    loop.loop();
}
