#include "TcpConnection.h"
#include "ReactorEventLoop.h"
#include "common/ByteBuffer.h"
#include "log/easylogging++.h"
#include "tcp/Channel.h"

#include <asm-generic/errno-base.h>
#include <cassert>
#include <cerrno>
#include <functional>
#include <string>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <utility>

TcpConnection::TcpConnection(ReactorEventLoop* loop, int fd)
  : fd_(fd)
  , running_(true)
  , context_(nullptr)
  , loop_(loop)
  , channel_(std::make_unique<Channel>(loop, fd))
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

    if (running_) {
        handleClose();
    }

    delete context_;
}

void TcpConnection::init()
{
    channel_->tie(shared_from_this());
    channel_->enableReading();
}

void TcpConnection::send(const std::string& msg)
{
    ByteBuffer<> buffer(msg.size() + 1);
    buffer.append(msg);

    loop_->runInLoop([this, &buffer] {
        sendInLoop(&buffer);
    });
}

void TcpConnection::send(ByteBuffer<>* buffer)
{
    loop_->runInLoop([this, buffer] {
        sendInLoop(buffer);
    });
}

void TcpConnection::send(std::shared_ptr<ByteBuffer<>> buffer)
{
    loop_->runInLoop([this, buffer] {
        sendInLoop(buffer);
    });
}

void TcpConnection::shutdown()
{
}

void* TcpConnection::context()
{
    return context_;
}

void TcpConnection::set_context(void* context)
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
    if (read_bytes <= 0) {
        handleClose();
        return;
    }

    if (receive_bytes_cb) {
        receive_bytes_cb(shared_from_this(), &read_buffer_);
    } else {
        FAT << "No receive_bytes_cb";
    }
}

void TcpConnection::handleWrite()
{
    loop_->assetInLoopThread();

    if (!channel_->isWriting()) {
        INF << fd_ << " dont care write buffer (closed) with lenth=" << write_buffer_.size();
        return;
    }

    if (!running_) {
        INF << fd_ << " handle write after not running" << write_buffer_.size();
        return;
    }

    auto write_bytes = write_buffer_.write_fd(fd_);
    TRA << "Write bytes=" << write_bytes << " in write loop";
    if (write_buffer_.size() == 0) {
        channel_->disableWriting();
    }
}

void TcpConnection::handleClose()
{
    loop_->assetInLoopThread();

    assert(running_);
    running_ = false;

    channel_->close();

    close(fd_);

    if (close_tcp_cb) {
        close_tcp_cb(fd_);
    }

    TRA << fd_ << " closed";
}

void TcpConnection::sendInLoop(ByteBuffer<>* buffer)
{
    loop_->assetInLoopThread();

    if (!running_) {
        return;
    }

    if (write_buffer_.size() > 0) {
        write_buffer_.append(*buffer);
        return;
    }

    if (buffer->write_fd(fd_) == -1) {
        if (errno != EAGAIN) {
            ERR << "write error with errno=" << errno << " " << strerror(errno);
            return;
        }
    }

    if (buffer->size() > 0) {
        write_buffer_.append(*buffer);
        channel_->enableWriting();
    }
}

void TcpConnection::sendInLoop(std::shared_ptr<ByteBuffer<>> buffer)
{
    sendInLoop(buffer.get());
}