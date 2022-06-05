#include <sys/timerfd.h>
#include <csignal>
#include "spdlog/spdlog.h"
#include "fm/EventLoop.h"

EventLoop eventLoop;

std::atomic_int a;

void timeout()
{
    spdlog::info("time out");
    eventLoop.quit();
}

void fun1()
{
    sleep(1);
    while(1) {
        eventLoop.runInLoop([](){
            spdlog::info("fun1");
            a++;
            usleep(100);
        });
    }
    spdlog::info("a : {}" , a);

}

void fun2()
{
    sleep(1);
    while(1) {
        eventLoop.runInLoop([](){
            spdlog::info("fun2");
            a++;
            usleep(100);
        });
    }
    spdlog::info("a : {}" , a);
}


int main()
{
    spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");

    std::thread t1(fun1);
    std::thread t2(fun2);
//    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
//                     TFD_NONBLOCK | TFD_CLOEXEC);
//    Channel channel(&eventLoop, timerfd);
//    channel.enableReading();
//    channel.setReadCallBack(timeout);
//
//    struct itimerspec howlong{};
//    bzero(&howlong, sizeof howlong);
//    howlong.it_value.tv_sec = 5;
//    ::timerfd_settime(timerfd, 0, &howlong, nullptr);
    eventLoop.runInLoop([](){
        spdlog::info("aaa");
    });

    spdlog::info("bbb");

    eventLoop.loop();

}
