#ifndef SOCK_H_
#define SOCK_H_

#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

void err_exit(const char* msg);

int Socket(int _domain, int _type, int _protocol);

void Bind(int _fd, const sockaddr* _addr, socklen_t _len);

void Listen(int _fd, int _n);

int Accept(int _fd, sockaddr* __restrict__ _addr, socklen_t* __restrict__ _addr_len);
ssize_t Write(int _fd, const void* _buf, size_t _n);

ssize_t Read(int _fd, void* _buf, size_t _nbytes);

/// @brief
/// @param addr
/// @param ip default: nullptr
/// @param port default: 0
void SetSockaddr(sockaddr_in* addr, const char* ip, uint16_t port);

int Epoll_create1(int _flags);

void Epoll_ctl(int _epfd, int _op, int _fd, epoll_event* _event);

int Epoll_wait(int _epfd, epoll_event* _events, int _maxevents, int _timeout);

std::string getSockInfo(const sockaddr_in* addr);

int EventFd(uint _count, int _flags);

void SetNonBlockingSocket(int _fd);

#endif  // SOCK_H_