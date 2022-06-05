#include <sys/timerfd.h>
#include <csignal>
#include "spdlog/spdlog.h"
#include "fm/EventLoop.h"
#include "fm/TcpServer.h"
#include "fm/TcpConnection.h"
#include "fm/EventLoopThread.h"
#include <iostream>
EventLoop eventLoop;

std::atomic_int a;

void newConnect(const TcpConnection::ptr &conn) {
    if (conn->isConnected()) {
        spdlog::info("new connect fd:{}", conn->getFd());
        a++;
    }else{
        spdlog::info("dis connect fd:{}", conn->getFd());
        a--;
    }
}

void newRead(const TcpConnection::ptr &conn,Buffer& buffer , int n) {
    std::string s(buffer.begin() + buffer.getReadIndex(),n);

    std::cout << "data:" << s << " n : "<< n  << std::endl;
    conn->Send(s);
    buffer.retrieve(n);
}

void Close(const TcpConnection::ptr &conn) {
    spdlog::info("Close");
}

void Error(const TcpConnection::ptr &conn) {
    spdlog::info("Error");
}


int main()
{

    spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [%@] [thread %t] %v");

    InetAddress inetAddress("127.0.0.1",9991, false);
    TcpServer tcpServer(&eventLoop, &inetAddress);
    tcpServer.setConnectCallBack(newConnect);
    tcpServer.setReadCallBack(newRead);
    tcpServer.setErrorCallBack(Error);
    tcpServer.setCloseCallBack(Close);
//    tcpServer.setThreadNums(4);
    tcpServer.start();
    eventLoop.loop();
}
