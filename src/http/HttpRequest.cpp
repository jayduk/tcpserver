#include "http/HttpRequest.h"

void HttpRequest::set_header(const std::string& key, const std::string& value)
{
    headers_[key] = value;
}

std::string HttpRequest::header(const std::string& key) const
{
    auto pheader = headers_.find(key);
    if (pheader != headers_.end())
    {
        return pheader->second;
    }
    return "";
}

std::string HttpRequest::uri() const
{
    return uri_;
}
std::string HttpRequest::method() const
{
    return method_;
}
Buffer* HttpRequest::inputBuffer()
{
    return &body_buffer_;
}
