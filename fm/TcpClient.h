//
// Created by Administrator on 2022/4/26.
//

#ifndef FM_TCPCLIENT_H
#define FM_TCPCLIENT_H

#include <memory>
#include "functional"
#include "EventLoop.h"
#include "Connector.h"
#include "TcpConnection.h"
class TcpClient {
public:
    TcpClient(EventLoop* l,InetAddress* i);
public:
    void setConnectCallBack(const TcpConnection::ConnectCallBack &connectCallBack);

    void setReadCallBack(const TcpConnection::ReadCallBack &readCallBack);

    void setCloseCallBack(const TcpConnection::CloseCallBack &closeCallBack);

    void setWriteCallBack(const TcpConnection::WriteCallBack &writeCallBack);

    void setErrorCallBack(const TcpConnection::ErrorCallBack &callBack);

    void connect();


    bool isConnected() const {
        if(conn_)return conn_->isConnected();
        return false;
    }

    const std::shared_ptr<TcpConnection>& getConn() const
    {
        return conn_;
    }

private:
    void handelConnect(int fd);
    EventLoop *loop;
    InetAddress *inetAddress;
    TcpConnection::ConnectCallBack connectCallBack;
    TcpConnection::ReadCallBack readCallBack;
    TcpConnection::CloseCallBack closeCallBack;
    TcpConnection::WriteCallBack writeCallBack;
    TcpConnection::ErrorCallBack errorCallBack;
    std::shared_ptr<Connector> connector;
    std::shared_ptr<TcpConnection> conn_;
//    std::shared_ptr<Channel> channel;
    static int s_id;
};


#endif //FM_TCPCLIENT_H
