#include <sys/timerfd.h>
#include "spdlog/spdlog.h"
#include "fm/EventLoop.h"

EventLoop eventLoop;

void timeout()
{
    spdlog::info("time out");
    eventLoop.quit();
}
int main()
{
    spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                     TFD_NONBLOCK | TFD_CLOEXEC);
    Channel channel(&eventLoop, timerfd);
    channel.enableReading();
    channel.setReadCallBack(timeout);

    struct itimerspec howlong{};
    bzero(&howlong, sizeof howlong);
    howlong.it_value.tv_sec = 5;
    ::timerfd_settime(timerfd, 0, &howlong, nullptr);

    eventLoop.loop();

}
