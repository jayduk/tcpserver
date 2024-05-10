#ifndef HTTPSTREAM_H
#define HTTPSTREAM_H

#include "common/ByteBuffer.h"
#include <cstddef>
#include <mutex>
#include <string>

class HttpStream
{
    ByteBuffer<> buffer_;
    std::mutex   mt_;

public:
    HttpStream()  = default;
    ~HttpStream() = default;

public:
    void   write(const std::string& data);
    void   write(const char* data, size_t len);
    size_t read(char* buffer, size_t len);

    void transfer_to(ByteBuffer<>* buffer);

    [[nodiscard]] size_t size() const;
};

#endif  // HTTPSTREAM_H
