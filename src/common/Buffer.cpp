#include "Buffer.h"
#include <algorithm>
#include <bits/types/struct_iovec.h>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <string>
#include <sys/uio.h>

#include "log/easylogging++.h"

Buffer::Buffer(size_t init_size)
  : buffer_(init_size)
  , read_index_(0)
  , write_index_(0)
  , cache_offset_(0)
{
}

size_t Buffer::writeableBytes() const
{
    return buffer_.size() - write_index_;
}

size_t Buffer::readableBytes() const
{
    return write_index_ - read_index_;
}

size_t Buffer::availableBytes() const
{
    return buffer_.size() - readableBytes();
}

void Buffer::ensureSpace(size_t len)
{
    if (writeableBytes() >= len)
        return;

    if (availableBytes() >= len)
    {
        size_t readable = readableBytes();
        std::copy(begin() + read_index_,
                  begin() + write_index_,
                  begin());
        read_index_  = 0;
        write_index_ = read_index_ + readable;
    }
    else
    {
        buffer_.resize(write_index_ + len);
    }
}

void Buffer::append(const void* data, size_t len)
{
    ensureSpace(len);
    memcpy(tail(), data, len);
    fill(len);
}

void Buffer::append(const std::string& data)
{
    append(data.c_str(), data.size());
}

size_t Buffer::readFromFd(int fd, int& _errno)
{
    char   extra_buffer[1024];
    size_t total_read_size = 0;

    while (true)
    {
        iovec iov[2];
        auto  buffer_writeable = writeableBytes();

        iov[0].iov_base = tail();
        iov[0].iov_len  = buffer_writeable;

        iov[1].iov_base = extra_buffer;
        iov[1].iov_len  = sizeof(extra_buffer);

        ssize_t read_size = ::readv(fd, iov, 2);

        if (read_size == -1)
        {
            if (errno == EINTR)
                continue;
            else if (errno == EAGAIN)
                return total_read_size;
            else
            {
                _errno = errno;
                return total_read_size;
            }
        }
        else
            total_read_size += read_size;

        if (read_size > (ssize_t)buffer_writeable)
        {
            fillAll();
            append(extra_buffer, read_size - buffer_writeable);
        }
        else
            fill(read_size);
    }
}

void Buffer::fill(size_t len)
{
    assert(writeableBytes() >= len);
    write_index_ += len;
}

void Buffer::fillAll()
{
    write_index_ = buffer_.size();
}

std::string Buffer::retrieveAllAsString()
{
    std::string str(peek(), tail());
    retrieveAll();
    return str;
}

size_t Buffer::tryReadString(size_t len, std::string* out)
{
    size_t size = len < readableBytes() ? len : readableBytes();

    (*out) = std::string(peek(), peek() + size);
    retrieve(size);
    return size;
}

void Buffer::retrieve(size_t len)
{
    assert(readableBytes() >= len);

    if (readableBytes() == len)
        retrieveAll();
    else
    {
        read_index_ += len;

        if (cache_offset_ > len)
            cache_offset_ -= len;
        else
            cache_offset_ = 0;
    }
}

void Buffer::retrieveUtil(const char* end)
{
    auto size = end - peek();

    if (size < 0 || (size_t)size > readableBytes())
    {
        WAR << "Buffer::retrieveUtil(const char* end) err";
        return;
    }

    retrieve(size);
}

void Buffer::retrieveAll()
{
    read_index_ = write_index_ = cache_offset_ = 0;
}

char* Buffer::begin()
{
    return &*buffer_.begin();
}
char* Buffer::end()
{
    return &*buffer_.end();
}
char* Buffer::peek()
{
    return &*(begin() + read_index_);
}
char* Buffer::tail()
{
    return &*(begin() + write_index_);
}

char* Buffer::findFirst(char ch, char* beg, char* end)
{
    auto target = std::find(beg, end, ch);
    return target == tail() ? nullptr : &*target;
}

char* Buffer::findFirst(char ch)
{
    return findFirst(ch, peek(), tail());
}

char* Buffer::cacheFindFirst(char ch)
{
    char* start = peek() + cache_offset_;
    char* res   = findFirst(ch, start, tail());
    if (res)
        cache_offset_ = 0;
    else
        cache_offset_ = readableBytes();

    return res;
}

char* Buffer::findFirst(const std::string& ch, char* beg, char* end)
{
    auto target = std::search(beg, end, ch.begin(), ch.end());
    return target == tail() ? nullptr : &*target;
}

char* Buffer::findFirst(const std::string& ch)
{
    return findFirst(ch, peek(), tail());
}

char* Buffer::cacheFindFirst(const std::string& ch)
{
    char* start = peek() + cache_offset_;
    char* res   = findFirst(ch, start, tail());
    if (res)
        cache_offset_ = 0;
    else
        cache_offset_ = readableBytes();

    return res;
}
