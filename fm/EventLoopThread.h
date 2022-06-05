//
// Created by Administrator on 2022/4/26.
//

#ifndef FM_EVENTLOOPTHREAD_H
#define FM_EVENTLOOPTHREAD_H

#include "EventLoop.h"
#include "spdlog/spdlog.h"
#include <thread>
#include <semaphore>
#include <condition_variable>
class EventLoopThread {
public:

    EventLoop* startLoop();
    ~EventLoopThread();
private:
    std::mutex mutex_;
    EventLoop* loop_;
    std::condition_variable cv;
    void func();

};


#endif //FM_EVENTLOOPTHREAD_H
