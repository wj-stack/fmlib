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
#include "sgame.h"

void newConnect(const TcpConnection::ptr &conn) {
    if (conn->isConnected()) {
        spdlog::info("client login");
    }
}

void sendMessage(const TcpConnection::ptr &conn,const std::string& str)
{
    uint32_t size = str.size();
//    spdlog::error("real size:{}",size);
    size = ntohl(size);
    std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>();
//    spdlog::error("size:{}",sizeof(uint32_t));
//    spdlog::error("ntohl:{}",size);
    char *psize = (char *) (&size);
    buffer->append(psize, sizeof(uint32_t));
    buffer->append(const_cast<char *>(str.c_str()), str.size());
//    spdlog::info("buffer:{}", buffer->getReadable());
//    spdlog::info("conn->Send(buffer)");
    conn->Send(buffer);
//    spdlog::info("buffer:{}", buffer->getReadable());
}

std::string encodeJson(const string& str,bool showMsg)
{
    Json::Value json;
    json["hero"] = false;
    json["msg"] = str;
    json["show"] = showMsg;
    return json.toStyledString();
}
std::string readHeroDataJson()
{
    Json::Value json;
    json["hero"] = true;
    json["show"] = false;
    for (int i = 0; i < 10; ++i) {
        Address v = firstHeroAddress + i * 0x19D0;
        int hero = 111;
        MemoryTools::preadv(pid, v, &hero, sizeof(int));
        int x = 5000;
        MemoryTools::preadv(pid, v + 0x10, &x, sizeof(int));
        int y = 1000;
        MemoryTools::preadv(pid, v + 0x18, &y, sizeof(int));
        int maxhp = 15000;
        MemoryTools::preadv(pid, v + 0x34, &maxhp, sizeof(int));
        int hp = 1000;
        MemoryTools::preadv(pid, v + 0x30, &hp, sizeof(int));
        int rank = 1;
        MemoryTools::preadv(pid, v + 0x8 , &rank, sizeof(int));
        Json::Value data;
        data["id"] = hero;
        data["maxhp"] = maxhp;
        data["hp"] = hp;
        data["x"] = x;
        data["y"] = y;
        data["rank"] = rank;
        json[to_string(i)] = Json::Value(data);

        if (hero < 0 || hero > 600 || x < -58000 || x > 58000 || rank < 1 || rank > 2 || maxhp < 1000 ||
            maxhp > 30000) {
            // 说明异常了，需要换一个首地址
//            fixAddress();
        }

    }
    spdlog::info(json.toStyledString());
    return json.toStyledString();
}
std::shared_ptr<std::thread>initHeroAddressThread;

std::mutex init_mutex;
void initHeroAddress(const TcpConnection::ptr &conn)
{
    lock_guard<mutex> lockGuard{init_mutex};
    initPid();
//    firstHeroAddress = 0;
    bool isfind = init();
//    Address isfind = segAddress();
//    firstHeroAddress = isfind;
    if (isfind) {
        sendMessage(conn, encodeJson("init success!" + to_string(isfind), true));
    }else
    {
        sendMessage(conn, encodeJson("init error! ",true));
    }
}

thread readHeroDataThread;
bool isReadHeroData;
void readHeroData(const TcpConnection::ptr &conn)
{
    sendMessage(conn, readHeroDataJson());
}

void handle(const TcpConnection::ptr &conn,vector<char> &buffer)
{
    std::string s(buffer.data(), buffer.size());
    if(s == "FIND_HERO_ADDRESS")
    {
        spdlog::info("FIND_HERO_ADDRESS");

//        initHeroAddressThread = std::make_shared<thread>(initHeroAddress, conn);
        thread t(initHeroAddress, conn);
        t.detach();

        // ... 寻找基址


    } else if (s == "GET_HERO_DATA") {

//        spdlog::info("GET_HERO_DATA");
        // .. 开始循环获取英雄坐标和HP
        isReadHeroData = true;
        readHeroData(conn); // echo模式
//        readHeroDataThread = thread(readHeroData, conn);
//        readHeroDataThread.detach();

    }else if(s == "STOP_HERO_DATA")
    {
        spdlog::info("STOP_HERO_DATA");
        isReadHeroData = false;
    }
}


thread_local uint32_t size;
const int type_size = sizeof(uint32_t);

void newRead(const TcpConnection::ptr &conn,Buffer& buffer , int n) {

    while (buffer.getReadable() >= type_size + size) {
        if (size == 0 && buffer.getReadable() >= type_size)
        {
            memcpy(&size, buffer.begin() + buffer.getReadIndex(), type_size);
            size = htonl(size); // 获取size
//            spdlog::info("all size: {}", size);
//            spdlog::info("buffer.getReadable: {}", buffer.getReadable());
        }

        if (buffer.getReadable() < type_size + size)break;

//        std::string s(buffer.begin() + buffer.getReadIndex() + type_size,buffer.getReadable() -  type_size);
        vector<char> vec(size);
        memcpy(vec.data(), buffer.begin() + buffer.getReadIndex() + type_size, size);
        handle(conn,vec);
        buffer.retrieve(size + type_size);
        size = 0;
//        spdlog::info("s: {}", s);
//        spdlog::info("size: {}", size);
    }

//    conn->Send(std::string('a',1024 * 1024 * 64));
//    buffer.retrieve(n);
}




void Close(const TcpConnection::ptr &conn) {
//    spdlog::info("Close");
    isReadHeroData = false;
}

void Error(const TcpConnection::ptr &conn) {

    isReadHeroData = false;
}


int main()
{

//    cout << readHeroDataJson();

    EventLoop eventLoop;
    InetAddress inetAddress(9991, false);
    TcpServer tcpServer(&eventLoop, &inetAddress);
    tcpServer.setConnectCallBack(newConnect);
    tcpServer.setReadCallBack(newRead);
    tcpServer.setErrorCallBack(Error);
    tcpServer.setCloseCallBack(Close);
    tcpServer.start();
    eventLoop.loop();


//    initPid();
//    init();
//    spdlog::info("init success");
//    showAllAddress();
//    init();
//    segAddress();

//    arm64-v8a
}

