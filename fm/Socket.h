//
// Created by wyatt on 2022/4/21.
//

#ifndef SOCKETFRAMEWORK_SOCKET_H
#define SOCKETFRAMEWORK_SOCKET_H

#include "InetAddress.h"
#include <string>
#include <arpa/inet.h>
#include <byteswap.h>
#include "spdlog/spdlog.h"
class Socket {
public:
    static bool bind(int fd, const InetAddress &listenAddress);

    Socket() = default;

    static int createSocket();

    static bool listen(int fd, int n);

    static int accept(int fd, InetAddress &inetAddress);

    static void close(int fd);

    static void toIp(const sockaddr *addr, char *buf, int size);

    static int accept(int sockfd, sockaddr_in6 *addr);


    static const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);

    static void fromIpPort(const char* ip, uint16_t port,
                             struct sockaddr_in* addr);


    static int getSocketError(int sockfd);

    static struct sockaddr_in6 getLocalAddr(int sockfd);

    static struct sockaddr_in6 getPeerAddr(int sockfd);
    static bool isSelfConnect(int sockfd);

    static void shutdownWrite(int sockfd);



};


#endif //SOCKETFRAMEWORK_SOCKET_H
