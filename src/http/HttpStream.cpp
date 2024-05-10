#include "HttpStream.h"
#include <cstddef>
#include <mutex>

void HttpStream::write(const std::string& data)
{
    std::unique_lock<std::mutex> lock(mt_);
    buffer_.append(data);
}

void HttpStream::write(const char* data, size_t len)
{
    std::unique_lock<std::mutex> lock(mt_);
    buffer_.append(data, len);
}

size_t HttpStream::read(char* buffer, size_t len)
{
    std::unique_lock<std::mutex> lock(mt_);
    return buffer_.retrieve_as_data(buffer, len);
}

void HttpStream::transfer_to(ByteBuffer<>* buffer)
{
    std::unique_lock<std::mutex> lock(mt_);
    buffer->append(buffer_);
    buffer_.retrieve_all();
}

size_t HttpStream::size() const
{
    return buffer_.size();
}