#ifndef __BYTEBUFFER_H__
#define __BYTEBUFFER_H__

#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <string>
#include <sys/uio.h>
#include <unistd.h>
#include <utility>
#include <vector>

#include "log/easylogging++.h"

template<int buffer_size>
struct ByteBufferIterator
{
    typedef std::random_access_iterator_tag iterator_category;
    /// The type "pointed to" by the iterator.
    typedef char value_type;
    /// Distance between iterators is represented as this type.
    typedef ptrdiff_t difference_type;
    /// This type represents a pointer-to-value_type.
    typedef value_type* pointer;
    /// This type represents a reference-to-value_type.
    typedef value_type& reference;

    typedef ByteBufferIterator<buffer_size>    self;
    typedef std::vector<value_type*>::iterator map_iterator;

public:
    map_iterator cur_node;
    value_type*  cur{};
    value_type*  first{};
    value_type*  last{};

    void set_node(map_iterator node)
    {
        cur_node = node;
        first    = *cur_node;
        last     = first + difference_type(buffer_size);
    }

    value_type& operator*()
    {
        return *cur;
    }

    value_type* operator->()
    {
        return &(operator*());
    }

    self& operator++()
    {
        if (++cur == last) {
            set_node(cur_node + 1);
            cur = first;
        }
        return *this;
    }

    self& operator--()
    {
        if (cur == first) {
            set_node(cur_node - 1);
            cur = last;
        }
        --cur;
        return *this;
    }

    self operator++(int) &
    {
        self tmp = *this;
        ++*this;
        return tmp;
    }

    self operator--(int) &
    {
        self tmp = *this;
        --*this;
        return tmp;
    }

    self& operator+=(difference_type n)
    {
        difference_type offset = n + (cur - first);
        if (offset >= 0 && offset < difference_type(buffer_size)) {
            cur += n;
        } else {
            difference_type node_offset = offset > 0 ? offset / difference_type(buffer_size)
                                                     : -difference_type((-offset - 1) / buffer_size) - 1;

            set_node(cur_node + node_offset);
            cur = first + (offset - node_offset * difference_type(buffer_size));
        }

        return *this;
    }

    self& operator-=(difference_type n)
    {
        return *this += -n;
    }

    self operator+(difference_type n) const
    {
        self tmp = *this;
        return tmp += n;
    }

    self operator-(difference_type n) const
    {
        self tmp = *this;
        return tmp += -n;
    }

    difference_type operator-(const self& rhs) const
    {
        return difference_type(buffer_size) * (cur_node - rhs.cur_node - 1) + (cur - first) + (rhs.last - rhs.cur);
    }

    value_type& operator[](difference_type n) const
    {
        return *(*this + n);
    }

    bool operator==(const self& rhs) const
    {
        return cur_node == rhs.cur_node && cur == rhs.cur;
    }

    bool operator!=(const self& rhs) const
    {
        return *this != rhs;
    }
};

template<int buffer_size = 1024>
class ByteBuffer
{
public:
    using iterator = ByteBufferIterator<buffer_size>;
    using map_type = std::vector<char*>;

    map_type map_;
    iterator begin_;
    iterator end_;

public:
    explicit ByteBuffer(size_t size = buffer_size);
    ~ByteBuffer();

    ssize_t read_fd(int fd);
    ssize_t send_fd(int fd);

    std::vector<std::pair<char*, int>> clips();

    void        retrieve_all();
    void        retrieve(size_t len);
    std::string retrieve_as_string();
    size_t      retrieve_as_data(char* data, size_t len);

    void append(const char* data, size_t len);

    template<int other_buffer_size>
    void append(ByteBuffer<other_buffer_size> buffer);

    iterator begin();
    iterator end();

    [[nodiscard]] size_t size() const;

private:
    char* alloc_buffer();
    void  free_buffer(char* buffer);

    [[nodiscard]] size_t reserve_space();

    iterator make_iterator(size_t offset);

    std::vector<char*> make_read_buffers(size_t size, size_t* extra_buffers_start);

    template<typename InputIterator>
    void append_buffers(InputIterator begin, InputIterator end);
    void append_buffers(size_t count);

    void ensure_space(size_t len);
};

template<int buffer_size>
ByteBuffer<buffer_size>::ByteBuffer(size_t size)
  : map_((size + buffer_size - 1) / buffer_size)
  , begin_()
  , end_()
{
    for (auto& it : map_) {
        it = alloc_buffer();
    }

    end_.set_node(map_.begin());
    end_.cur = map_.front();

    begin_ = end_;
}

template<int buffer_size>
ByteBuffer<buffer_size>::~ByteBuffer()
{
    for (auto buffer : map_) {
        free_buffer(buffer);
    }
}

template<int buffer_size>
ssize_t ByteBuffer<buffer_size>::read_fd(int fd)
{
    size_t read_buffer_size = 2048;

    bool    read_over        = false;
    bool    error            = false;
    ssize_t total_read_bytes = 0;

    while (!read_over) {
        size_t extra_buffers_start = 0;

        auto buffers     = make_read_buffers(read_buffer_size, &extra_buffers_start);
        read_buffer_size = std::min(read_buffer_size * 2, (size_t)65001);

        ssize_t buffered_size = end_.last - end_.cur;

        struct iovec vec[buffers.size() + 1];
        vec[0].iov_base = end_.cur;
        vec[0].iov_len  = buffered_size;
        for (size_t i = 0; i < buffers.size(); ++i) {
            vec[i + 1].iov_base = buffers[i];
            vec[i + 1].iov_len  = buffer_size;
        }
        buffered_size += buffers.size() * buffer_size;

        ssize_t read_bytes = readv(fd, vec, buffers.size() + 1);
        ssize_t space_left = reserve_space();

        read_over = buffered_size > read_bytes;

        if (read_bytes < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                error = true;
                INF << errno;
            }
        } else if (read_bytes <= space_left) {
            end_ += read_bytes;
        } else {
            size_t needed_nodes      = (read_bytes - space_left) / buffer_size + 1;
            size_t extra_buffers_end = std::min(extra_buffers_start + needed_nodes, buffers.size());

            append_buffers(buffers.begin() + extra_buffers_start, buffers.begin() + extra_buffers_end);
            append_buffers(needed_nodes - extra_buffers_end - extra_buffers_start);

            extra_buffers_start += needed_nodes;
            end_ += read_bytes;
        }

        for (size_t i = extra_buffers_start; i < buffers.size(); ++i) {
            free_buffer(buffers[i]);
        }

        total_read_bytes += read_bytes;
    }

    return error ? -1 : total_read_bytes;
}

template<int buffer_size>
ssize_t ByteBuffer<buffer_size>::send_fd(int fd)
{
    if (size() == 0) {
        return 0;
    }

    size_t node_count = end_.cur_node - begin_.cur_node + 1;

    ssize_t write_bytes = 0;

    if (node_count == 1) {
        write_bytes = write(fd, begin_.cur, end_.cur - begin_.cur);
    } else {
        struct iovec vec[node_count];

        vec[0].iov_base = begin_.cur;
        vec[0].iov_len  = begin_.last - begin_.cur;

        size_t i = 1;
        for (; i < node_count - 1; ++i) {
            vec[i].iov_base = *(begin_.cur_node + i);
            vec[i].iov_len  = buffer_size;
        }

        vec[i].iov_base = end_.first;
        vec[i].iov_len  = end_.cur - end_.first + 1;

        write_bytes = writev(fd, vec, node_count);
    }

    if (write_bytes > 0) {
        retrieve(write_bytes);
    }

    return write_bytes;
}

template<int buffer_size>
std::vector<std::pair<char*, int>> ByteBuffer<buffer_size>::clips()
{
    std::vector<std::pair<char*, int>> clips;

    if (begin().cur_node == end().cur_node) {
        clips.emplace_back(begin().cur, end().cur - begin().cur);
    } else {
        clips.emplace_back(begin().cur, begin().last - begin().cur);
        for (auto it = begin().cur_node + 1; it != end().cur_node; ++it) {
            clips.emplace_back(*it, buffer_size);
        }
        clips.emplace_back(end().first, end().cur - end().first);
    }
    return clips;
}

template<int buffer_size>
void ByteBuffer<buffer_size>::retrieve_all()
{
    for (size_t i = 1; i < map_.size(); i++) {
        free_buffer(map_[i]);
    }
    map_.erase(map_.begin() + 1, map_.end());

    begin_ = end_ = make_iterator(0);
}

template<int buffer_size>
void ByteBuffer<buffer_size>::retrieve(size_t len)
{
    if (len == size()) {
        retrieve_all();
    } else {
        begin_ += len;

        for (auto it = map_.begin(); it != begin_.cur_node; ++it) {
            free_buffer(*it);
        }

        size_t begin_offset = begin_.cur - begin_.first;
        size_t end_offset   = end_ - begin_ + begin_offset;

        //TODO: will change iterator begin and left ?
        map_.erase(map_.begin(), begin_.cur_node);

        begin_ = make_iterator(begin_offset);
        end_   = make_iterator(end_offset);
    }
}

template<int buffer_size>
std::string ByteBuffer<buffer_size>::retrieve_as_string()
{
    std::string str(begin_, end_);
    retrieve(size());
    return str;
}

template<int buffer_size>
size_t ByteBuffer<buffer_size>::retrieve_as_data(char* data, size_t len)
{
    len = std::min(len, size());
    std::copy(begin_, begin_ + len, data);
    retrieve(len);
    return len;
}

template<int buffer_size>
void ByteBuffer<buffer_size>::append(const char* data, size_t len)
{
    ensure_space(len);

    // size_t index = 0;
    // while (index < len) {
    //     size_t write_index = std::min(typename iterator::difference_type(len - index), end_.last - end_.cur);
    //     std::copy(data + index, data + index + write_index, end_.cur);
    //     index += write_index;
    //     end_ += write_index;
    // }

    std::copy(data, data + len, end_);
    end_ += len;
}

template<int buffer_size>
template<int other_buffer_size>
void ByteBuffer<buffer_size>::append(ByteBuffer<other_buffer_size> buffer)
{
    ensure_space(buffer.size());

    std::copy(buffer.begin(), buffer.end(), end_);
    end_ += buffer.size();
}

template<int buffer_size>
typename ByteBuffer<buffer_size>::iterator ByteBuffer<buffer_size>::begin()
{
    return begin_;
}

template<int buffer_size>
typename ByteBuffer<buffer_size>::iterator ByteBuffer<buffer_size>::end()
{
    return end_;
}

template<int buffer_size>
char* ByteBuffer<buffer_size>::alloc_buffer()
{
    return new char[buffer_size];
}

template<int buffer_size>
void ByteBuffer<buffer_size>::free_buffer(char* buffer)
{
    delete[] buffer;
}

template<int buffer_size>
size_t ByteBuffer<buffer_size>::reserve_space()
{
    return make_iterator(map_.size() * buffer_size - 1) - end_ + 1;
}

template<int buffer_size>
size_t ByteBuffer<buffer_size>::size() const
{
    return end_ - begin_;
}

template<int buffer_size>
typename ByteBuffer<buffer_size>::iterator ByteBuffer<buffer_size>::make_iterator(size_t offset)
{
    iterator it;
    it.set_node(map_.begin() + offset / buffer_size);
    it.cur = it.first + (offset % buffer_size);
    return it;
}

template<int buffer_size>
std::vector<char*> ByteBuffer<buffer_size>::make_read_buffers(size_t size, size_t* extra_buffers_start)
{
    std::vector<char*> buffers;

    size -= end_.last - end_.cur;
    for (auto it = end_.cur_node + 1; it != map_.end() && size > 0; ++it) {
        buffers.push_back(*it);
        size -= buffer_size;
    }

    *extra_buffers_start = buffers.size();
    while (size > 0) {
        buffers.push_back(alloc_buffer());
        size -= buffer_size;
    }

    return buffers;
}

template<int buffer_size>
template<typename InputIterator>
void ByteBuffer<buffer_size>::append_buffers(InputIterator begin, InputIterator end)
{
    size_t begin_offset = begin_.cur - begin_.first;
    size_t end_offset   = end_ - begin_ + begin_offset;

    for (auto it = begin; it != end; ++it) {
        map_.push_back(*it);
    }

    begin_ = make_iterator(begin_offset);
    end_   = make_iterator(end_offset);
}

template<int buffer_size>
void ByteBuffer<buffer_size>::append_buffers(size_t count)
{
    if (count == 0) {
        return;
    }

    size_t begin_offset = begin_.cur - begin_.first;
    size_t end_offset   = end_ - begin_ + begin_offset;

    while (count-- > 0) {
        map_.push_back(alloc_buffer());
    }

    begin_ = make_iterator(begin_offset);
    end_   = make_iterator(end_offset);
}

template<int buffer_size>
void ByteBuffer<buffer_size>::ensure_space(size_t len)
{
    if (reserve_space() > len) {
        return;
    }

    size_t extra_nodes = (len - reserve_space()) / buffer_size + 1;
    append_buffers(extra_nodes);
}

#endif  //_ByteBuffer_h_