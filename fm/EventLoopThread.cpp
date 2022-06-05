//
// Created by Administrator on 2022/4/26.
//

#include "EventLoopThread.h"

EventLoop *EventLoopThread::startLoop()    {
    loop_ = nullptr;
    std::thread t(&EventLoopThread::func, this);
    t.detach();
    {
        std::unique_lock<std::mutex> lock {mutex_};
        cv.wait(lock, [&]() {
            return loop_ != nullptr;
        });
    }
    return loop_;
}

void EventLoopThread::func()   {
    EventLoop eventLoop;
    {
        std::lock_guard<std::mutex> lockGuard{mutex_};

        loop_ = &eventLoop;
        cv.notify_one();
    }
    eventLoop.loop();
}

EventLoopThread::~EventLoopThread() {
    if (loop_)loop_->quit();
}
