//
// Created by Administrator on 2022/4/23.
//

#ifndef FM_UNTIL_H
#define FM_UNTIL_H

#include <sys/types.h>
#include <syscall.h>
#include <unistd.h>
#include <cstdint>
#include <chrono>
#include <cstring>
#include <sys/timerfd.h>

class Until {
public:
    static pid_t GetThreadId() {
        return syscall(SYS_gettid);
    }
    static uint64_t TimeAfter(uint64_t delay){
        return std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now() + std::chrono::microseconds(delay * 1000)).time_since_epoch().count();
    }
    static void CreateTimer(int timerfd, uint64_t delay) {
        struct itimerspec howlong{};
        bzero(&howlong, sizeof howlong);
        howlong.it_value.tv_sec = delay / 1000;
        howlong.it_value.tv_nsec = (delay % 1000) * 1000 * 1000;
        ::timerfd_settime(timerfd, 0, &howlong, nullptr);

    }
    static uint64_t Now()         {
        return std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    }
};


#endif //FM_UNTIL_H
