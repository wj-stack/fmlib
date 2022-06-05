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




std::string encodeRedisCommand(const std::string & command)
{
    std::stringstream ss;
    ss << command;
    std::string str;
    string res;
    int cnt = 0;

    while (ss >> str) {
        res += '$' + to_string(str.length()) + "\r\n";
        res +=  str + "\r\n";
        cnt++;
    }
    res = '*' + to_string(cnt)  + "\r\n"+ res;
    return res;
}

void test()
{
    cout << encodeRedisCommand("set name huge");
}

void handelConnectEvent(const TcpConnection::ptr &conn)
{
    if(conn->isConnected())
    {
        spdlog::info("redis connected");
        conn->Send(encodeRedisCommand("set name huge"));
        conn->Send(encodeRedisCommand("get name"));
    }else
    {
        spdlog::info("exit()");
    }
}

void handelReadEvent(const TcpConnection::ptr &conn, Buffer &buffer, int n) {
    std::string s(buffer.begin() + buffer.getReadIndex(),n);
    buffer.retrieve(n);
    spdlog::info(s);

}
void handelErrorEvent(const TcpConnection::ptr &conn)
{

}

int main()
{

    test();

    EventLoop eventLoop;
    InetAddress inetAddress("192.168.117.130", 6379);
    TcpClient tcpClient(&eventLoop, &inetAddress);
    tcpClient.setConnectCallBack(handelConnectEvent);
    tcpClient.setReadCallBack(handelReadEvent);
    tcpClient.setErrorCallBack(handelErrorEvent);
    tcpClient.connect();
    eventLoop.loop();

}

