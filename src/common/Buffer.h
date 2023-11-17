#ifndef UTIL_BUFFER_H_
#define UTIL_BUFFER_H_

#include <cstddef>
#include <cstring>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "log/easylogging++.h"

class BufferNoEnoughContentException : public std::exception
{
private:
public:
    BufferNoEnoughContentException() = default;
};

class Buffer
{
private:
    std::vector<char> buffer_;
    std::size_t       read_index_;
    std::size_t       write_index_;

    std::size_t cache_offset_;

public:
    explicit Buffer(size_t init_size = 1024);
    // Buffer(const Buffer& buffer)
    //   : buffer_(buffer.buffer_)
    //   , read_index_(buffer.read_index_)
    //   , write_index_(buffer.write_index_)
    // {
    //     WAR << "Buffer(const Buffer& buffer)";
    // }
    ~Buffer() = default;

public:
    [[nodiscard]] size_t writeableBytes() const;
    [[nodiscard]] size_t readableBytes() const;
    [[nodiscard]] size_t availableBytes() const;

    void ensureSpace(size_t len);

    void append(const void* data, size_t len);
    void append(const std::string& data);
    size_t readFromFd(int fd, int& _errno);

    void fill(size_t len);
    void fillAll();

    void retrieve(size_t len);
    void retrieveUtil(const char* end);
    void retrieveAll();

    std::string retrieveAllAsString();
    size_t      tryReadString(size_t len, std::string* out);

    char* begin();
    char* end();

    char* peek();
    char* tail();

    // template<typename T>
    // Buffer& operator<<(const T& rhs);

    // template<typename T>
    // Buffer& operator>>(T& rhs);

public:
    char* findFirst(char ch, char* beg, char* end);
    char* findFirst(char ch);
    char* cacheFindFirst(char ch);

    char* findFirst(const std::string& ch, char* beg, char* end);
    char* findFirst(const std::string& ch);
    char* cacheFindFirst(const std::string& ch);
};

// template<typename T>
// Buffer& Buffer::operator<<(const T& rhs)
// {
//     append(&rhs, sizeof(T));
//     return *this;
// }

// template<typename T>
// Buffer& Buffer::operator>>(T& rhs)
// {
//     if (readableBytes() < sizeof(rhs))
//         throw BufferNoEnoughContentException();

//     std::memcpy(&rhs, &*(buffer_.begin() + (long)read_index_), sizeof(T));

//     if (readableBytes() == sizeof(rhs))
//         read_index_ = write_index_ = 0;
//     else
//         read_index_ += sizeof(T);

//     return *this;
// }

#endif  // UTIL_BUFFER_H_