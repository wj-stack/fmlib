//
// Created by wyatt on 2022/4/21.
//

#include "InetAddress.h"
#include "Socket.h"
#include <iostream>
#include <arpa/inet.h>
#include <cstring>

static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;


InetAddress::InetAddress(uint16_t port, bool loopBackOnly, bool ipv6) {
    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    in_addr_t ip = loopBackOnly ? kInaddrLoopback : kInaddrAny;
    addr.sin_addr.s_addr = htobe32(ip);
    addr.sin_port = htons(port);
    printf("s_addr = %#x ,port : %#x\r\n", addr.sin_addr.s_addr, addr.sin_port);
}


int InetAddress::getStructSize() const {
    return sizeof(addr);
}

std::string InetAddress::toIp() const {
    char buf[64];
    Socket::toIp(getSocketAddress(), buf, sizeof(buf));
    return buf;
}

InetAddress::InetAddress(const std::string &ip, uint16_t portArg, bool ipv6)     {
    if (ipv6 || strchr(ip.c_str(), ':'))
    {
        bzero(&addrv6, sizeof addrv6);
        Socket::fromIpPort(ip.c_str(), portArg, (sockaddr_in*)&addrv6);
    }
    else
    {
        bzero(&addr, sizeof addr);
        Socket::fromIpPort(ip.c_str(), portArg, &addr);
    }
}

const sockaddr * InetAddress::getSocketAddress() const { return Socket::sockaddr_cast(&addrv6); }

void InetAddress::setSockAddrInet6(const sockaddr_in6 &addr6)  { addrv6 = addr6; }
