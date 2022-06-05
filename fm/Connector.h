//
// Created by Administrator on 2022/4/26.
//

#ifndef FM_CONNECTOR_H
#define FM_CONNECTOR_H

#include "EventLoop.h"
#include <functional>
#include <memory>
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
class Connector {
public:
    enum States { kDisconnected, kConnecting, kConnected };

    typedef std::function<void(int)> ConnectCallBack;
    explicit Connector(EventLoop* l,InetAddress* i);

    bool connect();
    void start();
    void stop();
    void setConnectCallBack(const ConnectCallBack & callBack);

private:
    void connecting(int fd);
    void retry(int fd);
    void handleWrite();
    void startInLoop();

    int fd_;
    EventLoop *loop;
    InetAddress *inetAddress;
    ConnectCallBack connectCallBack;
    std::shared_ptr<Channel> channel;
    States state_ = kDisconnected;
};


#endif //FM_CONNECTOR_H
