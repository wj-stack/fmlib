#include "Connector.h"
#include <sys/socket.h>
#include <unistd.h>
Connector::Connector(EventLoop *l, InetAddress *i) : loop(l)  , inetAddress(i) {

}

void Connector::start() {
    loop->runInLoop(std::bind(&Connector::startInLoop, this)); // FIXME: unsafe
}

void Connector::stop() {
    state_ = kDisconnected;
}

void Connector::setConnectCallBack(const Connector::ConnectCallBack &callBack) {
    connectCallBack = callBack;
}

bool Connector::connect() {
    auto &fd = fd_;
    if (fd) // 防止重用
    {
        ::close(fd);
        fd = 0;
    }
    fd = Socket::createSocket();
    spdlog::info(" Connector::connect createSocket fd:{}", fd);
    int ret = ::connect(fd, inetAddress->getSocketAddress(), static_cast<socklen_t>(sizeof(struct sockaddr_in6)));

//    spdlog::info("connect ret:{}", ret);
    int savedErrno = (ret == 0) ? 0 : errno;
    switch (savedErrno)
    {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
//            spdlog::warn("Transport endpoint is already connected");
            connecting(fd);
            break;

        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            retry(fd);
            break;

        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
//            LOG_SYSERR << "connect error in Connector::startInLoop " << savedErrno;
            spdlog::error("connect error in Connector::startInLoop {}", savedErrno);
            ::close(fd);
            break;

        default:
            spdlog::error("Unexpected error in Connector::startInLoop {}", savedErrno);
            ::close(fd);
            break;
    }
    return ret == 0;
}

void Connector::connecting(int fd) {
    state_ = kConnecting;
    if (channel)
    {
        channel->disableAll();
        channel->remove();
    }
    channel = std::make_shared<Channel>(loop, fd);
    channel->setWriteCallBack(std::bind(&Connector::handleWrite, this));
    channel->enableWriting();
}

void Connector::handleWrite() {
//    spdlog::info("Connector::handleWrite {}", state_);
    if (state_ == kConnecting)
    {
        int sockfd = channel->fd();
        int err = Socket::getSocketError(sockfd);
        if (err)
        {
//            spdlog::error("Connector::handleWrite - SO_ERROR");
            retry(sockfd);
        }
        else if (Socket::isSelfConnect(sockfd))
        {
            spdlog::error("Connector::handleWrite - Self connect");
            retry(sockfd);
        }
        else
        {
            state_ = kConnected;
//            spdlog::info("connectCallBack! {}" , (long&)connectCallBack);
            if(connectCallBack)connectCallBack(sockfd);
        }
    }
    else
    {
        // what happened?
        assert(state_ == kDisconnected);
    }
}

void Connector::retry(int sockfd)    {
    spdlog::warn("connect error , retry");
    ::close(sockfd);
    state_ = kDisconnected;
    loop->runAfter(1000,
                   std::bind(&Connector::startInLoop, this));
}

void Connector::startInLoop(){
    assert(loop->isInLoopThread());
    state_ = kDisconnected;
    connect();
}

