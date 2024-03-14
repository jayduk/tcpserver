#ifndef TCPSERVER_TCP_INETADDRESS_H_
#define TCPSERVER_TCP_INETADDRESS_H_

#include <cstdint>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

class InetAddress
{
private:
    sockaddr_in sockaddr_;
    socklen_t   socklen_;

public:
    InetAddress();
    explicit InetAddress(const char* ip, uint16_t port = 0);
    explicit InetAddress(uint16_t port);

    ~InetAddress() = default;

    [[nodiscard]] sockaddr* addr() const;
    [[nodiscard]] socklen_t len() const;
    socklen_t&              len();

    [[nodiscard]] std::string   ipString() const;
    [[nodiscard]] std::uint16_t port() const;

    [[nodiscard]] std::string addressString() const;
};

#endif  // TCPSERVER_TCP_INETADDRESS_H_