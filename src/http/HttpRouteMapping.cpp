#include "http/HttpRouteMapping.h"
#include <cassert>
#include <cstdlib>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

HttpRouteMapping::HttpRouteMapping()
  : roots(std::vector<RouteNode*>((int)HttpMethod::MAX_FLAG_))
  , default_fns(std::vector<HttpHandleFn>((int)HttpMethod::MAX_FLAG_, nullptr))
{
}

void HttpRouteMapping::add(HttpMethod method, const std::string_view& path, HttpHandleFn fn)
{
    assert((int)method >= 0);
    assert((int)HttpMethod::MAX_FLAG_ > (int)method);

    std::vector<std::string_view> parts = split(path);

    RouteNode* node = roots[(int)method];

    if (node == nullptr) {
        roots[(int)method] = node = new RouteNode("");
    }

    RouteNode* target = node->insert(parts, 0);

    if (target->fn != nullptr) {
        ERR << "route conflict: `" << path << "` and `" << target->pattern << "`";
        abort();
    }

    target->fn      = std::move(fn);
    target->pattern = path;
}

void HttpRouteMapping::add_default(HttpMethod method, HttpHandleFn fn)
{
    assert((int)method >= 0);
    assert((int)HttpMethod::MAX_FLAG_ > (int)method);

    default_fns[(int)method] = fn;
}

HttpHandleFn HttpRouteMapping::find(HttpMethod method, const std::string_view& path, std::map<std::string, std::string>* params)
{
    assert((int)method >= 0);
    assert((int)HttpMethod::MAX_FLAG_ > (int)method);

    std::vector<std::string_view> parts = split(path);

    RouteNode* node = roots[(int)method];

    std::vector<int> wild_indexs;
    auto             fn = node->find(parts, 0, wild_indexs, params);

    if (fn == nullptr) {
        return default_fns[(int)method];
    }

    return fn;
}

std::vector<std::string_view> HttpRouteMapping::split(const std::string_view& path)
{
    std::vector<std::string_view> parts;

    size_t start = 0;
    size_t end   = 0;

    while (end < path.size()) {
        if (path[end] == '?') {
            break;
        }
        if (path[end] == '/') {
            if (end > start) {
                parts.push_back(path.substr(start, end - start));
            }
            start = end + 1;
        }
        end++;
    }

    if (end > start) {
        parts.push_back(path.substr(start, end - start));
    }

    return parts;
}

RouteNode::RouteNode(const std::string_view& part)
  : part(part)
  , is_wild(part[0] == ':')
  , fn(nullptr)
{
}

RouteNode* RouteNode::insert(const std::vector<std::string_view>& parts, size_t index)
{
    if (index == parts.size()) {
        return this;
    }

    std::string_view part = parts[index];

    bool is_wild = part[0] == ':';
    if (is_wild) {
        if (!next.empty() && next.back()->is_wild) {
            return next.back()->insert(parts, index + 1);
        } else {
            auto* node = new RouteNode(part);
            next.push_back(node);

            return node->insert(parts, index + 1);
        }
    }

    for (auto node : next) {
        if (node->part == part) {
            return node->insert(parts, index + 1);
        }
    }

    auto* node = new RouteNode(part);

    if (!next.empty() && next.back()->is_wild) {
        next.insert(next.end() - 1, node);
    } else {
        next.push_back(node);
    }

    return node->insert(parts, index + 1);
}

HttpHandleFn RouteNode::find(const std::vector<std::string_view>& parts, int index, std::vector<int>& wild_indexs, std::map<std::string, std::string>* params)
{
    if (index == parts.size()) {
        if (fn != nullptr && !wild_indexs.empty()) {
            auto originals = HttpRouteMapping::split(pattern);
            for (auto i : wild_indexs) {
                params->insert({std::string(originals[i].substr(1)), std::string(parts[i])});
            }
        }
        return fn;
    }

    std::string_view part = parts[index];

    for (auto node : next) {
        if (node->part == part || node->is_wild) {
            if (node->is_wild) {
                wild_indexs.push_back(index);
            }
            auto fn = node->find(parts, index + 1, wild_indexs, params);
            if (fn != nullptr) {
                return fn;
            }
            if (node->is_wild) {
                wild_indexs.pop_back();
            }
        }
    }

    return nullptr;
}