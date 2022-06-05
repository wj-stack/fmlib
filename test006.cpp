#include <sys/timerfd.h>
#include <csignal>
#include "spdlog/spdlog.h"
#include "fm/EventLoop.h"
#include "fm/TcpServer.h"
#include "fm/TcpConnection.h"
#include <iostream>
EventLoop eventLoop;

std::atomic_int a;

void newConnect(const TcpConnection::ptr &conn) {
//    spdlog::info("new connect");
    if (conn->isConnected()) {
        a++;
    }else{
        a--;
    }
}

void newRead(const TcpConnection::ptr &conn,Buffer& buffer , int n) {

    std::cout << "data:" << buffer.begin() + buffer.getReadIndex() << " n : "<< n  << std::endl;
    std::string s(buffer.begin() + buffer.getReadIndex(),n);
//    conn->Send(std::string('a',1024 * 1024 * 64));
    buffer.retrieve(n);
}

void Close(const TcpConnection::ptr &conn) {
//    spdlog::info("Close");
}

void Error(const TcpConnection::ptr &conn) {
//    spdlog::info("Error");
}

void showA()
{
    while (1)
    {
        spdlog::info("client:{}", a);
        sleep(1);
    }
}

int main()
{
    spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [%@] [thread %t] %v");

    InetAddress inetAddress(9991, false);
    TcpServer tcpServer(&eventLoop, &inetAddress);
    tcpServer.setConnectCallBack(newConnect);
    tcpServer.setReadCallBack(newRead);
    tcpServer.setErrorCallBack(Error);
    tcpServer.setCloseCallBack(Close);
    tcpServer.start();
    std::thread t(showA);
    eventLoop.loop();

}
