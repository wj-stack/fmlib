//
// Created by Administrator on 2022/4/23.
//

#ifndef FM_EVENTLOOP_H
#define FM_EVENTLOOP_H


#include <thread>
#include "noncopyable.h"
#include "Channel.h"
#include "Poller.h"
#include "TimerQueue.h"
#include "spdlog/spdlog.h"
class EventLoop : public noncopyable{
public:
    EventLoop();
    ~EventLoop();
    void loop();
    void quit();
    bool isInLoopThread() const;
    void runInLoop(const Channel::EventCallBack &callBack);
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    void runAt(const Channel::EventCallBack &callBack);
    void runAfter(uint64_t delay, const Channel::EventCallBack &callBack);
private:
    pid_t threadId_;
    bool looping_{false};
    Poller poller;
    bool quit_ = false;
    std::vector<Channel::EventCallBack> callbacks_;
    int wakeFd_;
    Channel wakeChannel_;
    std::mutex mutex_;
    TimerQueue timerQueue;
    static int createEventFd();

    void wakeUp() const;
    void handleRead() const; // 处理唤醒，防止一直唤醒
    void removeInLoop(Channel *channel);
    void updateInLoop(Channel *channel);
};


#endif //FM_EVENTLOOP_H
