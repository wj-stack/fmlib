//
// Created by Administrator on 2022/4/26.
//

#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *loop): baseLoop(loop){

}

void EventLoopThreadPool::setThreadNum(int size) {
    if(isStart)return;
    eventLoop.resize(size);
    loopThread.resize(size, std::make_shared<EventLoopThread>());
}

EventLoop *EventLoopThreadPool::getNext()     {
    if ((int)eventLoop.size() == 0)
    {
        return baseLoop;
    }
    next = (next + 1) % (int)eventLoop.size();
    return eventLoop[next];
}

void EventLoopThreadPool::start() {
    isStart = true;
    for (int i = 0; i < eventLoop.size(); ++i) {
        eventLoop[i] = loopThread[i]->startLoop();
    }
}
