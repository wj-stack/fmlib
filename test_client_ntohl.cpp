#include <iostream>
#include "fm/TcpClient.h"
#include "spdlog/spdlog.h"
#include "fm/unpack.h"
using namespace std;

EventLoop eventLoop;
InetAddress inetAddress("192.168.186.128", 9991);
TcpClient tcpClient(&eventLoop, &inetAddress);



void sendMessage(const string& str)
{
    auto buffer = make_shared<Buffer>();
    unpack unpack;
    unpack.front(buffer);
    unpack.append(buffer, const_cast<char *>(str.c_str()), str.size());
    tcpClient.getConn()->Send(buffer);
}


void handelConnectEvent(const TcpConnection::ptr &conn)
{
    if(conn->isConnected())
    {
//        sendMessage("Hello world");
        spdlog::info("client isConnected");
    }
}

void handelReadEvent(const TcpConnection::ptr &conn, Buffer &buffer, int n) {
    std::string s(buffer.begin() + buffer.getReadIndex(),n);
    buffer.retrieve(n);
    spdlog::info(s);
}

void handelErrorEvent(const TcpConnection::ptr &conn)
{
    spdlog::info("error");
}

void handelCloseEvent(const TcpConnection::ptr &conn)
{
    spdlog::info("handelCloseEvent");
    tcpClient.connect(); // 断线重连
}

void runLoop()
{
    while (1)
    {
        int op = 0;
        string string1;
        cin >> op >> string1;
        if(op == 1)
        {
            cout << "send:" << string1 << endl;
            sendMessage(string1);
        }else if(op == 2)
        {
            int n;
            cin >> n;
            auto buffer = make_shared<Buffer>();
            char *a = new char[n];
            for (int i = 0; i < n; ++i) {
                a[i] = 'a' + (i % 26);
            }
            unpack::send(tcpClient.getConn(), a, n);
            delete[] a;
        }else if(op == 3)
        {
            int n;
            cin >> n;

            for (int i = 0; i < n; ++i) {
                auto buffer = make_shared<Buffer>();
//                char *a = new char[n];
//                for (int i = 0; i < 100; ++i) {
//                    a[i] = 'a' + (i % 26);
//                }
                unpack::send(tcpClient.getConn(), const_cast<char *>(string1.c_str()), string1.size());
//                delete[] a;
            }
        }

    }
}


//Buffer buffer;
//buffer.append(reinterpret_cast<char *>(&size), sizeof(size_t));
//buffer.append(const_cast<char *>(s.c_str()), s.length());
//conn->Send(buffer.begin() + buffer.getReadIndex(), buffer.getReadable());

int main()
{
    thread t(runLoop);

    tcpClient.setConnectCallBack(handelConnectEvent);
    tcpClient.setReadCallBack(handelReadEvent);
    tcpClient.setErrorCallBack(handelErrorEvent);
    tcpClient.setCloseCallBack(handelCloseEvent);
    tcpClient.connect();
    eventLoop.loop();


    return 0;
}