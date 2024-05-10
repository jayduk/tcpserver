#ifndef HTTP_HTTPROUTEMAPPING_H
#define HTTP_HTTPROUTEMAPPING_H

#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "http/common.h"
#include <cassert>
#include <cstddef>
#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

struct RouteNode;

using HttpHandleFn = std::function<void(HttpRequestPtr, HttpResponsePtr)>;
class HttpRouteMapping
{
    std::vector<RouteNode*>   roots;
    std::vector<HttpHandleFn> default_fns;

public:
    HttpRouteMapping();
    ~HttpRouteMapping() = default;

public:
    void add(HttpMethod method, const std::string_view& path, HttpHandleFn fn);
    void add_default(HttpMethod method, HttpHandleFn fn);

    HttpHandleFn find(HttpMethod method, const std::string_view& path, std::map<std::string, std::string>* params);

    static std::vector<std::string_view> split(const std::string_view& path);
};

struct RouteNode
{
    std::string pattern;
    std::string part;
    bool        is_wild;

    HttpHandleFn fn;

    std::vector<RouteNode*> next;

public:
    explicit RouteNode(const std::string_view& part);

    RouteNode* insert(const std::vector<std::string_view>& parts, size_t index);

    HttpHandleFn find(const std::vector<std::string_view>& parts, int index, std::vector<int>& wild_indexs, std::map<std::string, std::string>* params);
};

#endif  // HTTP_HTTPROUTEMAPPING_H