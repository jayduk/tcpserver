#include "sock.h"
#include <asm-generic/errno-base.h>
#include <cerrno>
#include <sys/types.h>

void err_exit(const char* msg)
{
    perror(msg);
    exit(errno);
}

int Socket(int _domain, int _type, int _protocol)
{
    int fd;
    if ((fd = socket(_domain, _type, _protocol)) < 0)
    {
        err_exit("create socket err");
    }
    return fd;
}

void Bind(int _fd, const sockaddr* _addr, socklen_t _len)
{
    if (bind(_fd, _addr, _len) < 0)
    {
        err_exit("bind err");
    }
}

void Listen(int _fd, int _n)
{
    if (listen(_fd, _n) < 0)
    {
        perror("listen err");
    }
}

int Accept(int _fd, sockaddr* __restrict__ _addr, socklen_t* __restrict__ _addr_len)
{
    int conn_fd;
    if ((conn_fd = accept(_fd, _addr, _addr_len)) < 0)
    {
        perror("accept err");
    }
    return conn_fd;
}

ssize_t Write(int _fd, const void* _buf, size_t _n)
{
    ssize_t write_len;
    while (_n > 0)
    {
        if ((write_len = write(_fd, _buf, _n)) <= 0)
        {
            if (write_len < 0 && errno == EINTR)  //中断
                continue;
            else
            {
                perror("write err");
                return -1;
            }
        }
        _n -= write_len;
        (uint8_t*&)_buf += write_len;
    }

    return static_cast<ssize_t>(_n);
}

ssize_t Read(int _fd, void* _buf, size_t _nbytes)
{
    ssize_t read_len = read(_fd, _buf, _nbytes);
    if (read_len == -1 && errno != EAGAIN)
    {
        perror("read error");
    }
    return read_len;
}

/// @brief
/// @param addr
/// @param ip default: nullptr
/// @param port default: 0
void SetSockaddr(sockaddr_in* addr, const char* ip, uint16_t port)
{
    memset(addr, 0, sizeof(*addr));
    addr->sin_port = htons(port);

    if (ip == nullptr)
    {
        return;
    }

    if (inet_aton(ip, &(addr->sin_addr)) == 0)
    {
        err_exit("ip err");
    }
}

int Epoll_create1(int _flags)
{
    int epoll_fd = -1;
    if ((epoll_fd = epoll_create1(_flags)) == -1)
    {
        err_exit("create epoll err");
    }
    return epoll_fd;
}

void Epoll_ctl(int _epfd, int _op, int _fd, epoll_event* _event)
{
    if (epoll_ctl(_epfd, _op, _fd, _event) == -1)
    {
        perror("epoll_ctl err");
    }
}

int Epoll_wait(int _epfd, epoll_event* _events, int _maxevents, int _timeout)
{
    int nfds = epoll_wait(_epfd, _events, _maxevents, _timeout);
    if (nfds == -1)
    {
        if (errno == EINTR)
        {
            perror("EINTR , may gdb ");
        }
        else
            err_exit("epoll wait err");
    }
    return nfds;
}

std::string getSockInfo(const sockaddr_in* addr)
{
    return std::string(inet_ntoa(addr->sin_addr)) + ":" + std::to_string(ntohs(addr->sin_port));
}

int EventFd(uint _count, int _flags)
{
    int event_fd = ::eventfd(_count, _flags);

    if (event_fd < 0)
    {
        err_exit("create eventfd err");
    }

    return event_fd;
}

void SetNonBlockingSocket(int _fd)
{
    int flags = fcntl(_fd, F_GETFL, 0);
    if (flags == -1)
    {
        err_exit("get file ctl err");
    }

    flags |= O_NONBLOCK;
    if (fcntl(_fd, F_SETFL, flags) == -1)
    {
        err_exit("set file ctl err");
    }
}