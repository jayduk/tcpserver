#ifndef COMMON_BYTEBUFFER_H_
#define COMMON_BYTEBUFFER_H_

#include <iterator>
#include <vector>

struct ByteBufferBlock
{
    int   size;
    char* data;
};

class ByteBufferIterator : public std::iterator<std::random_access_iterator_tag, char>
{
private:
    std::vector<char*> map_;

    char* cur_;

private:
    void increment()
    {
        ++cur_;
    }

    void decrement()
    {
        --cur_;
    }
};

class ByteBuffer
{
    typedef std::vector<char*> map_type;

private:
    map_type map;

public:
    ByteBuffer()  = default;
    ~ByteBuffer() = default;
};

#endif  // COMMON_BYTEBUFFER_H_