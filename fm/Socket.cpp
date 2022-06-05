//
// Created by wyatt on 2022/4/21.
//

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <csignal>
#include <unistd.h>
#include "Socket.h"
#include "spdlog/spdlog.h"

int Socket::createSocket() {
    int fd = ::socket(AF_INET,
                      SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);

    if (fd < 0) {
        SPDLOG_INFO("create socket error");
    }
    return fd;
}

bool Socket::bind(int fd, const InetAddress &listenAddress) {

    // 设置端口复用
    int on = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    if (::bind(fd, listenAddress.getSocketAddress(), listenAddress.getStructSize()) == -1) {
        spdlog::info("bind socket error: {}", strerror(errno));
        return false;
    }
    return true;
}

bool Socket::listen(int fd, int n) {

    if (::listen(fd, n) == -1) {
        spdlog::info("listen socket error: {}", strerror(errno));
        return false;
    }
    return true;
}

int Socket::accept(int fd, InetAddress &inetAddress) {
    struct sockaddr_in6 addr{};
    bzero(&addr, sizeof addr);
    int conn = accept(fd, &addr);
    inetAddress.setSockAddrInet6(addr);
    return conn;
}

int Socket::accept(int sockfd, struct sockaddr_in6 *addr) {
    socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
#if VALGRIND || defined (NO_ACCEPT4)
    int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
  setNonBlockAndCloseOnExec(connfd);
#else
    int connfd = ::accept4(sockfd, (struct sockaddr *) (addr),
                           &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
    if (connfd < 0) {
        int savedErrno = errno;
        spdlog::info("accept error: {}", strerror(errno));
        switch (savedErrno) {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: // ???
            case EPERM:
            case EMFILE: // per-process lmit of open file desctiptor ???
                // expected errors
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                spdlog::info("unexpected errors: {}", strerror(errno));
                break;
            default:
                spdlog::info("accept error: {}", strerror(errno));
                break;
        }
    }
    return connfd;
}

void Socket::toIp(const struct sockaddr *addr, char *buf, int size) {

    if (addr->sa_family == AF_INET) {
        const struct sockaddr_in *addr4 = (struct sockaddr_in *) (addr);
//        WYATT_LOG_ROOT_DEBUG() << "addr4" << addr4;
        ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
    } else if (addr->sa_family == AF_INET6) {
        assert(size >= INET6_ADDRSTRLEN);
        const struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *) (addr);
        ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
    }
}

void Socket::close(int fd) {
    ::close(fd);
}

const struct sockaddr *Socket::sockaddr_cast(const struct sockaddr_in6 *addr)     {
    return static_cast<const struct sockaddr*>((const void*)(addr));
}

void Socket::fromIpPort(const char *ip, uint16_t port, struct sockaddr_in *addr)    {
    addr->sin_family = AF_INET;
    addr->sin_port = bswap_16(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
    {
        spdlog::error("sockets::fromIpPort");
    }
}

int Socket::getSocketError(int sockfd)     {
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);

    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        return errno;
    }
    else
    {
        return optval;
    }
}

struct sockaddr_in6 Socket::getLocalAddr(int sockfd)     {
    struct sockaddr_in6 localaddr;
    bzero(&localaddr, sizeof localaddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
    sockaddr * p = const_cast<sockaddr *>(sockaddr_cast(&localaddr));
    if (::getsockname(sockfd, p, &addrlen) < 0)
    {
        spdlog::error("sockets::getLocalAddr");
    }
    return localaddr;
}

struct sockaddr_in6 Socket::getPeerAddr(int sockfd)     {
    struct sockaddr_in6 peeraddr;
    bzero(&peeraddr, sizeof peeraddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
    sockaddr * p = const_cast<sockaddr *>(sockaddr_cast(&peeraddr));
    if (::getpeername(sockfd, p, &addrlen) < 0)
    {
        spdlog::error("sockets::getPeerAddr");
    }
    return peeraddr;
}

bool Socket::isSelfConnect(int sockfd)     {
    struct sockaddr_in6 localaddr = getLocalAddr(sockfd);
    struct sockaddr_in6 peeraddr = getPeerAddr(sockfd);
    if (localaddr.sin6_family == AF_INET)
    {
        const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&localaddr);
        const struct sockaddr_in* raddr4 = reinterpret_cast<struct sockaddr_in*>(&peeraddr);
        return laddr4->sin_port == raddr4->sin_port
               && laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
    }
    else if (localaddr.sin6_family == AF_INET6)
    {
        return localaddr.sin6_port == peeraddr.sin6_port
               && memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof localaddr.sin6_addr) == 0;
    }
    else
    {
        return false;
    }
}

void Socket::shutdownWrite(int sockfd)     {
    if (::shutdown(sockfd, SHUT_WR) < 0)
    {
        SPDLOG_INFO("sockets::shutdownWrite");
    }
}
