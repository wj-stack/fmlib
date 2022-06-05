//
// Created by Administrator on 2022/4/26.
//

#include "TcpClient.h"


int TcpClient::s_id = 0;

void TcpClient::setConnectCallBack(const TcpConnection::ConnectCallBack &callBack) {
    connectCallBack = callBack;
}

void TcpClient::setReadCallBack(const TcpConnection::ReadCallBack &callBack) {
    readCallBack = callBack;
}

void TcpClient::setCloseCallBack(const TcpConnection::CloseCallBack &callBack) {
    closeCallBack = callBack;
}

void TcpClient::setWriteCallBack(const TcpConnection::WriteCallBack &callBack) {
    writeCallBack = callBack;
}

void TcpClient::setErrorCallBack(const TcpConnection::ErrorCallBack &callBack) {
    errorCallBack = callBack;
}

TcpClient::TcpClient(EventLoop *l, InetAddress *i) : loop(l), inetAddress(i) ,connector(std::make_shared<Connector>(loop,inetAddress)){
    connector->setConnectCallBack(std::bind(&TcpClient::handelConnect, this,std::placeholders::_1));
}

void TcpClient::connect() {
    connector->start();
}

void TcpClient::handelConnect(int fd) {

//    SPDLOG_INFO("fd: ");
//    SPDLOG_INFO(fd);
    auto ptr = std::make_shared<TcpConnection>(loop, fd, ++s_id);
    conn_ = ptr;
    Channel &channel = *ptr->getChannel();
    channel.enableReading();
    channel.setReadCallBack(std::bind(&TcpConnection::ReadHandel, ptr.get(), ptr)); // 先转发给readHandel然后转发给客户
    channel.setWriteCallBack(std::bind(&TcpConnection::WriteHandle, ptr.get(),ptr)); // 先转发给readHandel然后转发给客户

    ptr->setConnectCallBack(connectCallBack);
    ptr->setWriteCallBack(writeCallBack);
    ptr->setReadCallBack(readCallBack);
    ptr->setCloseCallBack(closeCallBack);
    ptr->setErrorCallBack(errorCallBack);
    ptr->setState(TcpConnection::CONNECTED);
    if (connectCallBack){
        connectCallBack(ptr); // 调用Connect回调函数
    }

}


