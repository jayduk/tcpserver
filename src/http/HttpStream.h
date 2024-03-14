#ifndef HTTPSTREAM_H
#define HTTPSTREAM_H

#include <cstddef>
#include <string>

class HttpStream
{
public:
    HttpStream()  = default;
    ~HttpStream() = default;

public:
    void append(const char* data, size_t len);
    void append(const std::string& data);

    [[nodiscard]] size_t size() const;
};

#endif  // HTTPSTREAM_H
