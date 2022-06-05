#include <sys/timerfd.h>
#include <csignal>
#include "spdlog/spdlog.h"
#include "fm/EventLoop.h"
#include "fm/TcpServer.h"
#include "fm/HttpServer.h"
#include "fm/TcpConnection.h"
#include "fm/EventLoopThread.h"
#include <iostream>
#include <memory>
#include "jsoncpp/include/json/json.h"
#include "MemoryTools/MemorySearchTools.h"
#include "LexicalCast/LexicalCast.h"
#include "fm/TcpClient.h"
#include <map>
#include "ThreadPool/ThreadPool.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "fm/unpack.h"


void handle(const TcpConnection::ptr &conn,vector<char> &buffer)
{

}

void newConnect(const TcpConnection::ptr &conn)
{


}

void newRead(const TcpConnection::ptr &conn,const char* data, int n) {
    std::string s(data, n);
    cout << s << endl;
}


void Close(const TcpConnection::ptr &conn) {
}

void Error(const TcpConnection::ptr &conn) {

}


int main()
{


    EventLoop eventLoop;
    InetAddress inetAddress(9991, false);
    TcpServer tcpServer(&eventLoop, &inetAddress);

    tcpServer.setConnectCallBack(newConnect);
    tcpServer.setReadCallBack(std::bind(unpack::adapterChar, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, newRead));

    tcpServer.setErrorCallBack(Error);
    tcpServer.setCloseCallBack(Close);
    tcpServer.start();
    eventLoop.loop();

}

