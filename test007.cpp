#include <sys/timerfd.h>
#include <csignal>
#include "spdlog/spdlog.h"
#include "fm/EventLoop.h"
#include "fm/TcpServer.h"
#include "fm/TcpConnection.h"
#include "fm/EventLoopThread.h"
#include <iostream>
#include "fm/TcpClient.h"


EventLoop eventLoop;

std::atomic_int a;
std::thread t;
void func(const TcpConnection::ptr &conn)
{
    while (conn->isConnected()) {
        spdlog::info("conn->isConnected");
        std::string  s;
        std::cin >>  s;
        conn->Send(s);
    }
}

void newConnect(const TcpConnection::ptr &conn) {
    spdlog::info("new connect");
    if (conn->isConnected()) {
        a++;
//        conn->Send("hello");
        t = std::thread (func,conn);
    }else{
        a--;
        spdlog::info("dis connect");
    }
}

void newRead(const TcpConnection::ptr &conn,Buffer& buffer , int n) {
    std::string s(buffer.begin() + buffer.getReadIndex(),n);

    std::cout << "data:" << s << " n : "<< n  << std::endl;
//    conn->Send(s);

//    conn->Send(std::string('a',1024 * 1024 * 64));
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

    InetAddress inetAddress("127.0.0.1",9991);
    TcpClient tcpClient(&eventLoop, &inetAddress);
    tcpClient.setConnectCallBack(newConnect);
    tcpClient.setCloseCallBack(Close);
    tcpClient.setErrorCallBack(Error);
    tcpClient.setReadCallBack(newRead);
    tcpClient.connect();

    eventLoop.loop();
}
