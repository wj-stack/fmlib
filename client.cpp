#include <sys/timerfd.h>
#include <csignal>
#include "spdlog/spdlog.h"
#include "fm/EventLoop.h"
#include "fm/TcpServer.h"
#include "fm/TcpConnection.h"
#include "fm/EventLoopThread.h"
#include <iostream>
#include "fm/TcpClient.h"
#include "fm/ThreadPool.h"


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

std::vector<std::shared_ptr<TcpClient>> tcpClient(100);
void newConnect(const TcpConnection::ptr &conn,int id) {
    if (conn->isConnected()) {
        a++;
//        spdlog::info("new connect fd:{}  id:{}", conn->getFd() , id);
        conn->shutdown();

    }else{
//        spdlog::info("dis connect fd:{}", conn->getFd());
//
//        spdlog::info("tcpClient[id]->isConnected() : {}", tcpClient[id]->isConnected());

        tcpClient[id]->connect();

    }
}

void newRead(const TcpConnection::ptr &conn,Buffer& buffer , int n) {
    std::string s(buffer.begin() + buffer.getReadIndex(),n);

    std::cout << "data:" << s << " n : "<< n  << std::endl;
    conn->Send(s);

//    conn->Send(std::string('a',1024 * 1024 * 64));
    buffer.retrieve(n);
}

void Close(const TcpConnection::ptr &conn) {
    a--;
//    spdlog::info("Close fd: {}" , conn->getFd());
}

void Error(const TcpConnection::ptr &conn) {
//    spdlog::info("Error");
}

InetAddress inetAddress("127.0.0.1",9991);
EventLoop* loop;

[[noreturn]] void func()
{
    for (int i = 0; i < 100; ++i) {
        tcpClient[i] = std::make_shared<TcpClient>(loop, &inetAddress);
        tcpClient[i]->setReadCallBack(newRead);
        tcpClient[i]->setConnectCallBack(std::bind(newConnect,std::placeholders::_1,i));
        tcpClient[i]->setCloseCallBack(Close);
        tcpClient[i]->connect();
//        spdlog::info("tcpClient[i]->connect() {}",i);
    }
    while (1)
    {
//        for (int i = 0; i < 100; ++i) {
//            if (!tcpClient[i]->isConnected()) {
//                tcpClient[i]->connect();
//            }
//        }
        spdlog::info("a: {}", a);
        sleep(1);
    }
}

int main()
{
    spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [%@] [thread %t] %v");
    EventLoopThread eventLoopThread;
    loop = eventLoopThread.startLoop();

    std::ThreadPool threadPool;
    threadPool.commit([&](){
        func();
    });
    while(1)sleep(1);
}
