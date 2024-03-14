#include "TcpConnection.h"
#include "Channel.h"
#include "ReactorEventLoop.h"
#include "common/ByteBuffer.h"
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

    ssize_t read_bytes = read_buffer_.read_fd(fd_);
    TRA << "read read_bytes=" << read_bytes << " in fd=" << fd_ << " errno = " << errno;
    if (read_bytes <= 0)
    {
        handleClose();
        return;
    }

    onReciveMessageCallback(shared_from_this(), &read_buffer_);
}

void TcpConnection::handleWrite()
{
    loop_->assetInLoopThread();

    if (!channel_->isWriting())
    {
        INF << fd_ << " dont care write buffer (closed) with lenth=" << write_buffer_.size();
        return;
    }

    auto write_bytes = write_buffer_.send_fd(fd_);
    TRA << "Write bytes=" << write_bytes << " in write loop";
    if (write_buffer_.size() == 0)
    {
        channel_->disableWriting();
    }
}

void TcpConnection::handleClose()
{
    loop_->assetInLoopThread();

    assert(running);
    running = false;

    channel_->close();

    close(fd_);

    if (onConnectionCloseCallback)
    {
        onConnectionCloseCallback(fd_);
    }

    TRA << fd_ << " closed";
}

void TcpConnection::sendInloop(const std::string& msg)
{
    loop_->assetInLoopThread();

    if (write_buffer_.size() > 0)
    {
        write_buffer_.append(msg.c_str(), msg.size());
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
                write_buffer_.append(&*start, msg.end() - start);
                TRA << "cache send message with length=" << msg.end() - start;
                channel_->enableWriting();
                return;
            } else if (errno == EINTR)
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