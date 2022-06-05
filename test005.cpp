#include <sys/timerfd.h>
#include <csignal>
#include "spdlog/spdlog.h"
#include "fm/EventLoop.h"
#include "fm/TimerQueue.h"

EventLoop eventLoop;
TimerQueue timerQueue(&eventLoop);

std::atomic_int a;

void timeout()
{
    spdlog::info("time out");
    eventLoop.quit();
}

void fun1()
{
    timerQueue.addTimer(timeout, 1000,false);
}

int main()
{
    spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [%@] [thread %t] %v");

    std::thread t(fun1);
//    timerQueue.addTimer(timeout, 1000,false);

    eventLoop.loop();

}
