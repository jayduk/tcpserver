#include "http/HttpResponse.h"
#include "common/ByteBuffer.h"
#include "http/common.h"
#include <chrono>
#include <iomanip>
#include <string>

using time_point = std::chrono::system_clock::time_point;
std::string serializeTimePoint(const time_point& time, const std::string& format)
{
    std::time_t tt = std::chrono::system_clock::to_time_t(time);
    std::tm     tm = *std::gmtime(&tt);  //GMT (UTC)
    //std::tm tm = *std::localtime(&tt); //Locale time-zone, usually UTC by default.
    std::stringstream ss;
    ss << std::put_time(&tm, format.c_str());
    return ss.str();
}

HttpResponse::HttpResponse()
  : version_{HttpVersion::Http1_1}
  , status_code_{200}
  , reason_phrase_{"OK"}
  , headers_{}
  , is_chunked_(false)
  , is_header_sent_(false)
{
    headers_["Server"] = "http-server";
    headers_["Date"]   = serializeTimePoint(std::chrono::system_clock::now(), "%a, %d %b %Y %H:%M:%S GMT");
}

void HttpResponse::set_status_code(int code, const std::string& phrase)
{
    status_code_   = code;
    reason_phrase_ = phrase;
}

void HttpResponse::set_header(const std::string& key, const std::string& value)
{
    headers_[key] = value;
}

HttpStream HttpResponse::body_buffer()
{
    return body_buffer_;
}

void HttpResponse::flush()
{
    set_chunked();
    ByteBuffer<> buffer;
    to_bytebuffer(&buffer);

    conn_->send(&buffer);
}

void HttpResponse::to_bytebuffer(ByteBuffer<>* buffer)
{
    if (!is_header_sent_) {
        buffer->append(HttpVersionToString(version_));
        buffer->append(" ");
        buffer->append(std::to_string(status_code_));
        buffer->append(" ");
        buffer->append(reason_phrase_);
        buffer->append("\r\n");

        if (!is_chunked_) {
            headers_["Content-Length"] = std::to_string(body_buffer_.size());
        }

        for (const auto& [key, value] : headers_) {
            buffer->append(key);
            buffer->append(": ");
            buffer->append(value);
            buffer->append("\r\n");
        }

        buffer->append("\r\n");

        is_header_sent_ = true;
    }

    // if (is_chunked_) {
    //     buffer->append(std::to_string(body_buffer_.size(), 16));
    //     buffer->append("\r\n");
    // }

    // buffer->append(body_buffer_);
}

void HttpResponse::set_chunked()
{
    if (is_chunked_) {
        return;
    }
    is_chunked_ = true;

    headers_["Transfer-Encoding"] = "chunked";
}