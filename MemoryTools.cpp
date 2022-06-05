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

EventLoop eventLoop;

int numA = 12321412;

static std::string ToHex(long long x) {
    stringstream ss;
    ss << hex << x;
    std::string ans;
    ss >> ans;
    return ans;
}

template<class T>
static T ToDec(const std::string& str)
{

    T tmp;
    sscanf(str.c_str(), "%llx", &tmp);
    return tmp;
}

int main()
{

    vector<MemPage> pages;
    shared_ptr<MemoryTools> memoryTools;
    int cnt = 0;
    int g_pid = getpid();
    InetAddress inetAddress(9991, false);
    HttpServer httpServer(&eventLoop, &inetAddress);

    // 获取所有进程ID 无参数
    httpServer.Get("/getAllPid", [](RequestHeader &requestHeader, ResponseHeader &responseHeader) {
        auto allPid = MemoryTools::getAllPID();
        Json::Value json;
        for (auto &v: allPid) {
            json[v.first] = v.second;
        }
        responseHeader.setData(json.toStyledString());
    });

    // 获取指定进程的内存页 参数: pid （无指定则默认是本进程）
    httpServer.Get("/getPages", [&](RequestHeader &requestHeader, ResponseHeader &responseHeader) {
        requestHeader.getParam();
        int pid = lexical_cast<int>(requestHeader.getParam("pid", to_string(g_pid)));
        MemoryTools::getMemPage(pid, pages);
        if (pages.empty())return;
        vector<std::map<string, string>> res;

        Json::Value json;

        for (auto &i: pages) {
            Json::Value page;
            page["start"] = ToHex(i.start);
            page["end"] = ToHex(i.end);
            page["flags"] = i.flags;
            page["name"] = i.name;
            json.append(page);
            cout << i << endl;
        }
        responseHeader.setData(json.toStyledString());
    });

    // 初始化 ， 参数： pid 默认本进程 ， 在搜素前一定要初始话，否则memoryTools没有实例
    httpServer.Get("/init", [&](RequestHeader &requestHeader, ResponseHeader &responseHeader) {
        spdlog::info("requestHeader.getParam(\"pid\", to_string(g_pid)): {}",
                     requestHeader.getParam("pid", to_string(g_pid)));
        int pid = lexical_cast<int>(requestHeader.getParam("pid", to_string(g_pid)));
        spdlog::info("init pid:{}  getpid: {} g_pid {}", pid,getpid(),g_pid);
        memoryTools = std::make_shared<MemoryTools>(pid);
        MemoryTools::getMemPage(pid, pages);
        cnt = 0;
        Json::Value json;
        json["pages"] = pages.size();
        responseHeader.setData(json.toStyledString());

    });



//    XX(char)
//    XX(short)
//    XX(int)
//    XX(long)
//    XX(long long)
//    XX(float)
//    XX(double)
    // 首次扫描，扫描所有可读的内存，并返回总的个数  参数 ： type （扫描类型，默认是 int ， XX里面的是参数）
    httpServer.Get("/first", [&](RequestHeader &requestHeader, ResponseHeader &responseHeader) {

//            MemoryTools::getMemPage(getpid(), pages);
//            memoryTools.reset(new MemoryTools(getpid()));
        Json::Value json;
        std::string type = (requestHeader.getParam("type", "int"));
        // 扫描类型
        spdlog::info("type: {}", type);


#define XX(T)           if (type == #T) {\
                size_t size = memoryTools->FirstSearch<T>(pages);\
                json["size"] = size;\
            };
        XX(char)
        XX(short)
        XX(int)
        XX(long)
        XX(long long)
        XX(float)
        XX(double)
#undef XX
        cnt = 1;
        responseHeader.setData(json.toStyledString());

    });

    // 二次扫描，只对于使用过first后使用， 参数： type（扫描类型，默认是int） ，value(默认是null,如果有参数，则 新值和value比较)
    // func: 默认是 isEqual ， 过滤出相等的
    httpServer.Get("/second", [&](RequestHeader &requestHeader, ResponseHeader &responseHeader) {


        std::string type = (requestHeader.getParam("type", "int"));
        std::string value = (requestHeader.getParam("value", "null"));
        std::string func = (requestHeader.getParam("func", "isEqual"));
        spdlog::info("type: {} value {} func {}", type, value, func);
        size_t size = 0;

#define XX(T, F)           if(type == #T)\
            {\
                if (value == "null") {  \
                    if(func == #F){     \
                    size = memoryTools->SecondSearch<T>(&MemoryTools::F<T>);\
                    }                    \
                }else{\
                    T v = lexical_cast<T>(value);                    \
                    if(func == #F){     \
                    size = memoryTools->SecondSearch<T>(&MemoryTools::F<T>,&v);\
                    }                 \
            }}


#define XXX(F)  XX(char,F)\
            XX(short,F)\
            XX(int,F)\
            XX(long,F)\
            XX(long long,F)\
            XX(float,F)\
            XX(double,F)

        XXX(isNotEqual)
        XXX(isEqual)
        XXX(isBigger)
        XXX(isSmaller)

#undef XXX

#undef XX
        Json::Value json;
        json["size"] = size;
        cnt = 2;
        cout << size << endl;
        cout << numA << endl;
        cout << &numA << endl;
        responseHeader.setData(json.toStyledString());

    });

    // 三次扫描及以上都是用这个函数， 参数： count （指定上一次扫描的次数，默认会自动填写，每次搜素+1）
    // type: 扫描类型
    // value ： 比较的值
    // func: 比较函数
    httpServer.Get("/third", [&](RequestHeader &requestHeader, ResponseHeader &responseHeader) {


        int count = lexical_cast<int>(requestHeader.getParam("count", to_string(cnt)));
        std::string type = (requestHeader.getParam("type", "int"));
        std::string value = (requestHeader.getParam("value", "null"));
        std::string func = (requestHeader.getParam("func", "isEqual"));
        size_t size = 0;

#define XX(T, F)           if(type == #T)\
            {\
                if (value == "null") {  \
                    if(func == #F){     \
                    size = memoryTools->ThirdSearch<T>(&MemoryTools::F<T>,count);\
                    }                    \
                }else{\
                    T v = lexical_cast<T>(value);                    \
                    if(func == #F){     \
                    size = memoryTools->ThirdSearch<T>(&MemoryTools::F<T>,count,&v);\
                    }                 \
            }}


#define XXX(F)  XX(char,F)\
            XX(short,F)\
            XX(int,F)\
            XX(long,F)\
            XX(long long,F)\
            XX(float,F)\
            XX(double,F)

        XXX(isNotEqual)
        XXX(isEqual)
        XXX(isBigger)
        XXX(isSmaller)

#undef XXX

#undef XX

        cout << size << endl;
//            resp->json.push_back({{"size", size}});
        Json::Value json;
        json["size"] = size;
        cnt++;
        responseHeader.setData(json.toStyledString());
    });


    // 查看扫描出的结果
    // count : 查看第 count 次的扫描结果
    // skip ： 超过 skip 条数据则不显示， 防止过度，默认1000
    httpServer.Get("/dump", [&](RequestHeader &requestHeader, ResponseHeader &responseHeader) {


        int count = lexical_cast<int>(requestHeader.getParam("count", to_string(cnt)));
        int skip = lexical_cast<int>(requestHeader.getParam("skip", "1000")); // 默认1000条

//        std::vector<map<std::string,std::string> > vec;
        Json::Value json;
        memoryTools->dump<int>(count, [&](const Address & address,const int& oldValue){
            if(json.size() > skip)return;
            Json::Value mp;
            spdlog::info("addr: {}", address);
            mp["address"] = ToHex(address);
            mp["oldValue"] = to_string(oldValue);
            json.append(mp);
        }, false);
        int c = 0;
        memoryTools->dump<int>(count, [&](const Address & address,const int& newValue){
            if(c > skip)return;
            json[c++]["newValue"] = to_string(newValue);
        }, true);

        responseHeader.setData(json.toStyledString());
    });


    // 读取指定进程地址
    // pid ： 默认本机
    // address : 地址 ， 需要一个16进制的数值
    // type : 读取类型 默认是int
    // size ： 顺序读取的个数 默认是1
    httpServer.Get("/read", [&](RequestHeader &requestHeader, ResponseHeader &responseHeader) {
        int pid = lexical_cast<int>(requestHeader.getParam("pid", to_string(g_pid)));
        spdlog::info("get Param address: {}", requestHeader.getParam("address", ToHex((long long) &numA)));
        auto address = ToDec<long long>(requestHeader.getParam("address", ToHex((long long)&numA)));
        spdlog::info("read address: {}", address);
        std::string type = requestHeader.getParam("type", "int");
        auto len = lexical_cast<size_t>(requestHeader.getParam("size", "1"));
        Json::Value json;

#define XX(T)            if(type == #T)\
            {\
                size_t size = len * sizeof(T);\
                vector<T> buff(len);\
                MemoryTools::preadv(pid,address,buff.data(),size); \
                for(int i = 0 ; i < buff.size() ; i ++){     \
                      json[i] = ToHex(buff[i]);      \
                }                       \
            }

        XX(char)
        XX(short)
        XX(int)
        XX(long)
        XX(long long)
        XX(float)
        XX(double)
#undef XX

        spdlog::info("address: {}", address);
        responseHeader.setData(json.toStyledString());
    });

    // 写数据
    // pid
    // address : 16进制
    // type
    // value : 10进制的值
    httpServer.Get("/write", [&](RequestHeader &requestHeader, ResponseHeader &responseHeader) {
        int pid = lexical_cast<int>(requestHeader.getParam("pid", to_string(g_pid)));
        auto address = ToDec<long long>(requestHeader.getParam("address", ToHex((long)&numA)));
        std::string type = requestHeader.getParam("type", "int");
//            auto len = lexical_cast<size_t>(req->GetParam("size", "1"));
        std::string value = requestHeader.getParam("value", "100");
        Json::Value json;
#define XX(T)            if(type == #T)\
            {                          \
                T v; \
                if(type=="char"){      \
                v =  lexical_cast<int>(value);                      \
                }else{                 \
                v = lexical_cast<T>(value);                       \
                }\
                MemoryTools::pwritev(  \
                pid,address,&v,1);\
                json = ToHex(v);\
            }

        XX(char)
        XX(short)
        XX(int)
        XX(long)
        XX(long long)
        XX(float)
        XX(double)
#undef XX
        spdlog::info("numA: {}", numA);
        responseHeader.setData(json.toStyledString());
//        return 200;
    });

    httpServer.start();

    eventLoop.loop();
}

//
//-DCMAKE_TOOLCHAIN_FILE=/home/wyatt/android-ndk-r23b/build/cmake/android.toolchain.cmake
//-DCMAKE_SYSTEM_NAME=Android
//-DANDROID_ABI="x86"
//-DANDROID_PLATFORM=android-21
//-DCMAKE_ANDROID_NDK=/home/wyatt/android-ndk-r23b
//-DCMAKE_SYSTEM_VERSION=21
//-DCMAKE_ANDROID_NDK_TOOLCHAIN_VERSION=clang
//-DANDROID_NATIVE_API_LEVEL=21