//
// Created by wyatt on 2022/4/21.
//

#ifndef SOCKETFRAMEWORK_ACCEPTOR_H
#define SOCKETFRAMEWORK_ACCEPTOR_H

#include "InetAddress.h"
#include "EventLoop.h"
#include "Channel.h"
#include <functional>
#include "Socket.h"

class Acceptor {
public:
    typedef std::function<void(int, const InetAddress &)> callback; // fd + InetAddress

    Acceptor(EventLoop *_loop, InetAddress *listen);

    void listen();

    bool listening() const;

    void setNewConnectCallBack(const callback &cb);

    void handleRead();

private:
    InetAddress *listenAddr;
    EventLoop *loop;
    bool isListening{};
    int sockfd;
    Socket m_socket;
    Channel channel;
    callback m_cb;
};


#endif //SOCKETFRAMEWORK_ACCEPTOR_H
