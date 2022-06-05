//
// Created by wyatt on 2022/4/20.
//

#ifndef SOCKETFRAMEWORK_TimerQueue_H
#define SOCKETFRAMEWORK_TimerQueue_H

#include <set>
#include "Timer.h"
#include "Channel.h"
#include <functional>
#include <memory>

class EventLoop;

class TimerQueue {
public:

    struct TimerEvent {
        bool operator()(const std::shared_ptr<Timer> timer1, const std::shared_ptr<Timer> timer2);
    };

    std::set<std::shared_ptr<Timer>, TimerEvent> timer_set;

    explicit TimerQueue(EventLoop *p);

    ~TimerQueue();

    int addTimer(const std::function<void()> &cb, uint64_t delay, bool recycle);

    void cancel(int id);

private:
     void TimeOut();

    void readTimerfd();

    EventLoop *loop;
    int timerfd;
    Channel channel;
};


#endif //SOCKETFRAMEWORK_TimerQueue_H
