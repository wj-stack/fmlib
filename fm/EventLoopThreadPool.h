//
// Created by Administrator on 2022/4/26.
//

#ifndef FM_EVENTLOOPTHREADPOOL_H
#define FM_EVENTLOOPTHREADPOOL_H

#include "EventLoop.h"
#include "EventLoopThread.h"
#include <vector>
#include <memory>
class EventLoopThreadPool {
public:
    EventLoopThreadPool(EventLoop *loop);
    EventLoop* getNext();
    void start();
    void setThreadNum(int size);
private:
    EventLoop *baseLoop;
    int next = 0;
    std::vector<std::shared_ptr<EventLoopThread>> loopThread;
    std::vector<EventLoop*> eventLoop;
    bool isStart = false;
};


#endif //FM_EVENTLOOPTHREADPOOL_H
