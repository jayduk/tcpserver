#include "TcpConnection.h"
#include "Channel.h"
#include "ReactorEventLoop.h"
#include "log/easylogging++.h"
#include "sock.h"
#include <asm-generic/errno-base.h>
#include <bits/types/struct_iovec.h>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <functional>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <thread>
#include <unistd.h>

TcpConnection::TcpConnection(ReactorEventLoop* loop, int fd, bool edge_mode)
  : fd_(fd)
  , running(true)
  , context_(nullptr)
  , loop_(loop)
  , channel_(std::make_unique<Channel>(loop, fd, edge_mode))
{
    channel_->readableCallback_ = [this] {
        handleRead();
    };
    channel_->writeableCallback_ = [this] {
        handleWrite();
    };
    channel_->closeCallback_ = [this] {
        handleClose();
    };
}

TcpConnection::~TcpConnection()
{
    INF << fd_ << " ~tcpconnection()";

    if (running)
        handleClose();
}

void TcpConnection::init()
{
    channel_->tie(shared_from_this());

    channel_->enableReading();
}

void TcpConnection::send(const std::string& msg)
{
    loop_->runInLoop(&TcpConnection::sendInloop, this, msg);
}

void TcpConnection::shutdown()
{
}

std::any& TcpConnection::context()
{
    return context_;
}

void TcpConnection::set_context(const std::any& context)
{
    context_ = context;
}

void TcpConnection::set_context(std::any&& context)
{
    context_ = context;
}

int TcpConnection::fd() const
{
    return fd_;
}

void TcpConnection::handleRead()
{
    loop_->assetInLoopThread();

    if (!readAllMessage())
    {
        handleClose();
        return;
    }
    INF << fd_ << " read message with size=" << read_buffer_.readableBytes() << "->" << std::string(read_buffer_.peek(), read_buffer_.tail());

    onReciveMessageCallback(shared_from_this(), &read_buffer_);
}

void TcpConnection::handleWrite()
{
    loop_->assetInLoopThread();

    if (!channel_->isWriting())
    {
        INF << fd_ << " dont care write buffer (closed) with lenth=" << write_buffer_.readableBytes();
        return;
    }

    while (write_buffer_.readableBytes() > 0)
    {
        ssize_t write_bytes = write(fd_, write_buffer_.peek(), write_buffer_.readableBytes());

        if (write_bytes == -1)
        {
            if (errno == EAGAIN)
            {
                return;
            }
            else if (errno == EINTR)
                continue;
            else
            {
                ERR << fd_ << " Write error with errno=" << errno;
                return;  //TODO: write error}
            }
        }
        write_buffer_.retrieve(write_bytes);
        TRA << "send message in handleWrite() with length=" << write_bytes << " leave data with size=" << write_buffer_.readableBytes();

        if (write_buffer_.readableBytes() > 0)
            return;
    }

    channel_->disableWriting();
}

void TcpConnection::handleClose()
{
    loop_->assetInLoopThread();

    assert(running);
    running = false;

    channel_->close();

    close(fd_);

    if (onConnectionCloseCallback)
        onConnectionCloseCallback(fd_);

    TRA << fd_ << " closed";
}

void TcpConnection::sendInloop(const std::string& msg)
{
    loop_->assetInLoopThread();

    if (write_buffer_.readableBytes() > 0)
    {
        write_buffer_.append(msg);
        return;
    }

    auto start = msg.begin();
    while (start != msg.end())
    {
        ssize_t write_bytes = write(fd_, &*start, msg.end() - start);

        if (write_bytes == -1)
        {
            if (errno == EAGAIN)
            {
                write_buffer_.append(std::string(start, msg.end()));
                TRA << "cache send message with length=" << msg.end() - start;
                channel_->enableWriting();
                return;
            }
            else if (errno == EINTR)
                continue;
            else
            {
                ERR << "Write error " << errno;
                return;  //TODO: write error}
            }
        }
        start += write_bytes;

        TRA << "send message in loop with length=" << write_bytes;
    }
}

bool TcpConnection::readAllMessage()
{
    char extra_buffer[1024];

    while (true)
    {
        iovec iov[2];
        auto  buffer_writeable = read_buffer_.writeableBytes();

        iov[0].iov_base = read_buffer_.tail();
        iov[0].iov_len  = buffer_writeable;

        iov[1].iov_base = extra_buffer;
        iov[1].iov_len  = sizeof(extra_buffer);

        ssize_t read_size = ::readv(fd_, iov, 2);

        if (read_size == -1)
        {
            if (errno == EINTR)
                continue;
            else if (errno == EAGAIN)
            {
                return true;
            }
            else
            {
                ERR << fd_ << " read with size=-1 errno=" << errno;
                return false;
            }
        }

        if (read_size == 0)
            return false;

        if (read_size > (ssize_t)buffer_writeable)
        {
            read_buffer_.fillAll();
            read_buffer_.append(extra_buffer, read_size - buffer_writeable);
        }
        else
        {
            read_buffer_.fill(read_size);
        }
    }
}