#include "http/HttpRequest.h"
#include "http/common.h"
#include "log/easylogging++.h"
#include <string>

void HttpRequest::set_header(const std::string& key, const std::string& value)
{
    headers_[key] = value;
}

std::string HttpRequest::header(const std::string& key) const
{
    auto it = headers_.find(key);
    if (it != headers_.end()) {
        return it->second;
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
HttpStream* HttpRequest::input_stream()
{
    return &body_buffer_;
}

std::map<std::string, std::string>* HttpRequest::path_params()
{
    return &path_params_;
}

std::string HttpRequest::path_param(const std::string& key) const
{
    auto it = path_params_.find(key);
    if (it != path_params_.end()) {
        return it->second;
    }
    return "";
}

void HttpRequest::set_method(HttpMethod method)
{
    method_ = method;
}
void HttpRequest::set_version(HttpVersion version)
{
    version_ = version;
}
