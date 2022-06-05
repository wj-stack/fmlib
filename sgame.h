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


//auto logger = spdlog::basic_logger_mt("basic_logger", "./basic-log.txt");


map<std::string, int> offset // 偏移
        {
                {"hero",  0x0},
                {"x",     0x10},
                {"y",     0x18},
                {"maxhp", 0x34},
                {"wg",    0x38},
                {"wk",    0x3c},
                {"dw",    0x8} // 红队或者蓝队 1 为蓝，2为红
        };

typedef pair<int,int> PII;
map<std::string, PII> value // 小乔
        {
                {"hero", {100,800}},
                {"x",{-58000,58000}},
                {"y",{-58000,58000}},
                {"maxhp",{100,25000}},
                {"wg",{10,2500}},
                {"wk",{10,2500}},
                {"dw",{1,2}}
        };


auto pid = MemoryTools::getPID("com.tencent.tmgp.sgame");

bool check(long address) {
    int x;
    bool flag = true;
    for (auto &v: offset) {
        MemoryTools::preadv(pid, address + v.second,&x,sizeof (x));
        if (x < value[v.first].first || x > value[v.first].second ) {
            flag = false;
            break;
        }
    }
    return flag;
}

void show(long address,bool log = true) {
    int x;

//    if(log)logger->info("address: {0:x}", address);
    spdlog::info("address: {0:x}", address);
    for (auto &v: offset) {
        MemoryTools::preadv(pid, address + v.second,&x,sizeof (x));
//        if(log)logger->info("{}:{}", v.first, x);
//        else
        {
            spdlog::info("{}:{}", v.first, x);
        }

    }
}


std::mutex _mutex;
vector<Address> addrs;
set<Address> watch;
bool isFind = false;
Address firstHeroAddress = 0;

bool findFirstAddress()
{
    set<Address> tmp;
    {
        lock_guard<mutex> lockGuard{_mutex};
        tmp = watch;
    }
    for (auto& addr: tmp) {
        int cnt = 0;
        for (int j = 0; j < 15; ++j) {
            Address v = addr + j * 0x19D0;
            if (tmp.find(v) != tmp.end()) {
                cnt++;
            }
        }
        if(cnt >= 8)
        {
            firstHeroAddress = addr;
            return true;
        }
    }
    return false;
}

atomic_int success{0};

void readAddress(MemPage& page)
{
    {
        lock_guard<mutex> lockGuard{_mutex};
        cout << page << endl;
    }

    if (page.flags[0] != 'r' || page.flags[1] != 'w' || strcmp(page.name,"[anon:libc_malloc]") != 0 || page.start < 0 || isFind) {
        success++;
        return;
    }


    int buffSize = page.end - page.start;


    if(buffSize <= 0)return;
    char *buff = new char[buffSize];
    MemoryTools::preadv(pid,page.start,buff ,buffSize);

    for (int i = 0; i < buffSize / sizeof(int); i++) {

        Address addr = page.start + i * sizeof(int);

        bool flag = true;
        for (const auto& v: offset) {
            if (i * sizeof(int) + (v.second) > buffSize - sizeof(int)) continue;
            int val = *(int*)(buff + i * sizeof(int) + (v.second)); // value
            if (val < value[v.first].first || val > value[v.first].second ) {
                flag = false;
                break;
            }
        }
        if (flag) {
            // 找到一个可能是人物坐标的基址
            {
                lock_guard<mutex> lockGuard{_mutex};
                watch.insert(addr);
            }
        }

    }

    if(findFirstAddress())
    {
        isFind = true;
    }
    success++;
    delete[] buff;
}


pair<Address,Address> getSoAddress(vector<MemPage> &pages, const std::string &name) {
    Address minv = -1;
    Address maxv = -1;

    for(auto& page:pages)
    {
        string s = page.name;
        if (s.find(name) != string::npos) {
            if (minv == -1)
            {
                minv = page.start;
            }
            if(maxv == -1)
            {
                maxv = page.end;
            }
            minv = std::min(minv, page.start);
            maxv = std::max(maxv, page.end);
        }
    }
    return {minv, maxv};
}

pair<Address, Address> address_range;
void scanLibGameCore(MemPage& page) //
{
//    {
//        lock_guard<mutex> lockGuard{_mutex};
//        cout << page << endl;
//    }


    string s = page.name;

    if (address_range.first > page.start || page.end > address_range.second || isFind) {
        success++;
        return;
    }

    {
        lock_guard<mutex> lockGuard{_mutex};
        cout << page << endl;
    }

    int buffSize = page.end - page.start;


    if(buffSize <= 0)
    {
        success++;
        return;
    }
    char *buff = new char[buffSize];
    MemoryTools::preadv(pid,page.start,buff ,buffSize);

    // 扫描 libGameCore

    for (int i = 0; i < buffSize / sizeof(int); i++) {

        Address addr = page.start + i * sizeof(int);
        Address addr2 = *(Address*)(buff + i * sizeof(int)); // value ： 二级地址，判断二级地址是否正确
        // 判断addr2是否可读

        Address addr3;
        MemoryTools::preadv(pid, addr2, &addr3, sizeof(Address));
        // check addr3 是否满足条件，满足则就是人物坐标

        int num;
        MemoryTools::preadv(pid, addr3 + 0x14, &num, sizeof(num)); // 10

        int t;
        MemoryTools::preadv(pid, addr3 + 0xC, &t, sizeof(t)); // 10

        if (num == 10 && t == 66) {

            {
                lock_guard<mutex> lockGuard{_mutex};
                spdlog::info("chazhi: {0} ", i * sizeof(int));
                spdlog::info("addr: {0:x} addr2: {1:x} addr3:{2:x}", addr, addr2, addr3);
                firstHeroAddress = addr3 + 0xC0;
                isFind = true;
            }

        }
//        5F305F30
//        MemoryTools::preadv(pid,addr2,buff ,buffSize);


    }
    success++;
    delete[] buff;
}

void initPid()
{
    pid = MemoryTools::getPID("com.tencent.tmgp.sgame");
}

threadpool _threadpool(8);

bool init()
{

#ifdef __LP64__
    spdlog::info("x64");
#else
    spdlog::info("x32");
#endif

    initPid();
    watch.clear();
    isFind = false;
    MemoryTools memoryTools(pid);
    vector<MemPage> pages;
    MemoryTools::getMemPage(pid, pages);
    address_range = getSoAddress(pages, "libGameCore.so");
    spdlog::info("begin addr :{} end addr : {}", address_range.first, address_range.second);
    success = 0;
    for(auto& page:pages)
    {
//        _threadpool.commit(readAddress, page);
        scanLibGameCore(page);
    }

    while (success != pages.size()) {
        sleep(3);
    }
    spdlog::info("firstHeroAddress: {0:x}", firstHeroAddress);
    spdlog::info("watch size: {0:x}", watch.size());
    spdlog::info("_threadpool.getTaskSize(): {}", _threadpool.getTaskSize());
    if (isFind)
    {
        return true;
    }
    return false;
}


void showAllAddress()
{
    while (true)
    {
        for(auto& v : watch)
        {
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
            spdlog::info("hero id:{0} x:{1} y:{2} maxhp:{3} rank:{4} address:{5:x}", hero, x, y, maxhp,rank, v);
        }
        findFirstAddress();
        spdlog::info("firstHeroAddress: {0:x}", firstHeroAddress);
        sleep(5);
    }
}



Address segAddress()
{
    initPid();
    MemoryTools memoryTools(pid);
    vector<MemPage> pages;
    MemoryTools::getMemPage(pid, pages);

    Address addr2;
    Address x = 0x63f1a69000 + 0xAABA80;
    Address v = 0;
    MemoryTools::preadv(pid, x, &v, sizeof(Address));
    spdlog::info("v: {}", v);
    Address a;
    MemoryTools::preadv(pid, v, &a, sizeof(Address));
    spdlog::info("a: {}", a);


    int cnt = 0;
    Address ans = 0;
    for(auto& page:pages)
    {
        string s = page.name;
        if(s.find("libGameCore.so") != string::npos)
        {
            cout << page << endl;
            cnt++;
            if (cnt == 3)
            {
                // 第三个

                Address addr = page.start + 0x1055D0; // 1055D0 人机  0x128724 排位/匹配

                Address addr2;
                MemoryTools::preadv(pid, addr, &addr2, sizeof(int));

                Address addr3;
                MemoryTools::preadv(pid, addr2, &addr3, sizeof(int));

                if (addr3)
                {
                    spdlog::info("addr3: {0}", addr3);
                    ans = addr3;
                }

            }
        }
    }
    return ans + 0xC0;

}

