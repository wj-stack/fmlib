//
// Created by wyatt on 2022/4/20.
//
#include "EventLoop.h"
#include "TimerQueue.h"
#include "Until.h"
#include <sys/timerfd.h>
#include <memory>
#include "spdlog/spdlog.h"
bool TimerQueue::TimerEvent::operator()(const std::shared_ptr<Timer> timer1, const std::shared_ptr<Timer> timer2) {
    if (timer1->getTimeStamp() != timer2->getTimeStamp())return timer1->getTimeStamp() < timer2->getTimeStamp();
    return timer1 < timer2;
}

void TimerQueue::TimeOut() {
//    spdlog::info("TimerQueue::TimeOut() time out");

    readTimerfd(); // 清除这个事件，要不然会一直重复出现 LT模式

    while (!timer_set.empty() && (*timer_set.begin())->getTimeStamp() <= Until::Now()) {
        auto &timer = (*timer_set.begin());
        loop->runInLoop(std::bind(&Timer::run,timer.get()));
        if (timer->isRecycle()) {
            timer->reset();
            timer_set.insert(timer);
        }
        timer_set.erase(timer_set.begin());
    }
}

TimerQueue::TimerQueue(EventLoop *p) : loop(p), timerfd(::timerfd_create(CLOCK_MONOTONIC,
                                                                             TFD_NONBLOCK | TFD_CLOEXEC)),
                                           channel(loop, timerfd) {
    channel.setReadCallBack(std::bind(&TimerQueue::TimeOut,this));
    channel.enableReading();

}

TimerQueue::~TimerQueue() {
    ::close(timerfd);
}

int TimerQueue::addTimer(const std::function<void()> &cb, uint64_t delay, bool recycle) {
    auto p = std::make_shared<Timer>(cb, delay, recycle);
    timer_set.insert(p);
    loop->runInLoop(std::bind(&Until::CreateTimer, timerfd, delay));
//    spdlog::info("addTimer() timerfd:{}",timerfd);
    return p->getId();
}

void TimerQueue::cancel(int id) {
    for(auto it = timer_set.begin() ; it != timer_set.end() ; it++)
    {
        if ((*it)->getId() == id) {
            timer_set.erase(it);
        }
    }
}

void TimerQueue::readTimerfd() {
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    if (n != sizeof howmany) {
        spdlog::error("TimerQueue::handleRead() reads {} bytes instead of 8", n);
    }
}
