#include "InetAddress.h"
#include "sock.h"
#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

InetAddress::InetAddress()
  : sockaddr_({})
  , socklen_(sizeof(sockaddr_))
{
    memset(&sockaddr_, 0, socklen_);
}

InetAddress::InetAddress(const char* ip, uint16_t port)
  : InetAddress()
{
    SetSockaddr(&sockaddr_, ip, port);
}
InetAddress::InetAddress(uint16_t port)
  : InetAddress()
{
    SetSockaddr(&sockaddr_, nullptr, port);
}

sockaddr* InetAddress::addr() const
{
    return (sockaddr*)&sockaddr_;
};

socklen_t InetAddress::len() const
{
    return socklen_;
};

socklen_t& InetAddress::len()
{
    return socklen_;
};

std::string InetAddress::ipString() const
{
    return inet_ntoa(sockaddr_.sin_addr);
}
std::uint16_t InetAddress::port() const
{
    return ntohs(sockaddr_.sin_port);
}

std::string InetAddress::addressString() const
{
    return ipString() + ":" + std::to_string(port());
}