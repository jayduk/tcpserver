#include "Channel.h"
#include <sys/epoll.h>

#include <utility>

#include "log/easylogging++.h"

Channel::Channel(ReactorEventLoop* loop, int fd, bool edge_mode)
  : loop_(loop)
  , index_(0)
  , sockfd_(fd)
  , event_(0)
  , revent_(0)
  , tied_(false)
{
    if (edge_mode)
        event_ |= EPOLLET;
}

Channel::~Channel()
{
    INF << fd() << " ~Channel()";
}

void Channel::handleEvent()
{
    loop_->assetInLoopThread();

    if (tied_ && tie_.expired())
    {
        WAR << "Channel tie has expired";
        return;
    }

    auto guard = tie_.lock();
    handleEventWithGuard();
}

void Channel::tie(const std::shared_ptr<void>& t)
{
    tie_  = t;
    tied_ = true;
}

void Channel::enableReading()
{
    loop_->assetInLoopThread();

    event_ |= EPOLLIN | EPOLLPRI;

    loop_->updateChannel(this);
    index_ = kToMod;

    // INF << "enableReading fd: " << fd();
}

void Channel::enableWriting()
{
    loop_->assetInLoopThread();

    event_ |= EPOLLOUT;

    loop_->updateChannel(this);
    index_ = kToMod;
}

void Channel::disableWriting()
{
    event_ &= ~EPOLLOUT;
    loop_->updateChannel(this);
}

bool Channel::isWriting() const
{
    return event_ & EPOLLOUT;
}

int Channel::fd() const
{
    return sockfd_;
}

uint32_t Channel::event() const
{
    return event_;
}

int Channel::index() const
{
    return index_;
}

void Channel::set_index(int index)
{
    index_ = index;
}

void Channel::set_revent(uint32_t event)
{
    revent_ = event;
}

void Channel::close()
{
    index_ = kToDel;
    event_ = 0;
    loop_->updateChannel(this);
}

std::string Channel::eventToString() const
{
    std::ostringstream oss;
    if (revent_ & EPOLLIN)
        oss << "EPOLLIN ";
    if (revent_ & EPOLLPRI)
        oss << "EPOLLPRI ";
    if (revent_ & EPOLLOUT)
        oss << "EPOLLOUT ";
    if (revent_ & EPOLLHUP)
        oss << "EPOLLHUP ";
    if (revent_ & EPOLLRDHUP)
        oss << "EPOLLRDHUP ";
    if (revent_ & EPOLLERR)
        oss << "EPOLLERR ";

    return oss.str();
}

void Channel::handleEventWithGuard()
{
    TRA << fd() << " trigger epoll event: " << eventToString();

    if ((revent_ & EPOLLHUP) && !(revent_ & EPOLLIN) && closeCallback_)
    {
        closeCallback_();
    }

    if (revent_ & (EPOLLIN | EPOLLPRI) && readableCallback_)
    {
        readableCallback_();
    }

    if (revent_ & (EPOLLOUT) && writeableCallback_)
    {
        writeableCallback_();
    }
}