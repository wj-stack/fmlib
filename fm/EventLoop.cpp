//
// Created by Administrator on 2022/4/23.
//

#include <sys/poll.h>
#include <sys/eventfd.h>
#include "EventLoop.h"
#include "Until.h"

thread_local EventLoop *t_loopInThisThread = nullptr;

EventLoop::EventLoop() : threadId_(Until::GetThreadId()), wakeFd_(createEventFd()), wakeChannel_(this, wakeFd_) ,
                         timerQueue((this)) {
    spdlog::info("threadId_:{}  loop:{}", threadId_, (long)this);
    if (!t_loopInThisThread) {
        t_loopInThisThread = this;
        wakeChannel_.setReadCallBack(std::bind(&EventLoop::handleRead, this));
        wakeChannel_.enableReading();
    } else if (t_loopInThisThread != this) {
        spdlog::error("当前线程已绑定loop");
    }
}

EventLoop::~EventLoop() {
    assert(!looping_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    spdlog::info("looping_: {}" , looping_);
    assert(!looping_);
    assert(isInLoopThread());
    looping_ = true;
    Poller::ChannelList activeChannelList;
    std::vector<Channel::EventCallBack> callback;
    while (!quit_) {
        activeChannelList.clear();
        int n = poller.poll(-1, &activeChannelList);
        for (auto &channel: activeChannelList) {
            channel.second->handle();
        }
//        if (n > 0) spdlog::info("poller event size:{}", n);
        {
            std::lock_guard<std::mutex> lockGuard{mutex_};
            std::swap(callback, callbacks_);
        }
        if (!callback.empty())
        {

            for(auto& i:callback)
            {
                i();
            }
            callback.clear();
            {
                std::lock_guard<std::mutex> lockGuard{mutex_};
                std::swap(callback, callbacks_);
            }
        }
    }

    spdlog::info("Event loop_ {0:x} stop looping", (long) this);
    looping_ = false;
}

bool EventLoop::isInLoopThread() const {
//    spdlog::info("cur thread: {} my thread:{}", Until::GetThreadId(), threadId_);
    return threadId_ == Until::GetThreadId();
}

void EventLoop::updateChannel(Channel *channel) {
    runInLoop(std::bind(&EventLoop::updateInLoop, this, channel));
}

void EventLoop::removeChannel(Channel *channel) {
    runInLoop(std::bind(&EventLoop::removeInLoop, this, channel));
}

void EventLoop::quit() {
    assert(looping_);
    quit_ = true;
    wakeUp();

}

void EventLoop::runInLoop(const Channel::EventCallBack &callBack) {
    if (isInLoopThread()) {
        callBack();
    } else {
        {
            std::lock_guard<std::mutex> lockGuard{mutex_};
            callbacks_.push_back(callBack);
        }
        wakeUp();
    }

}

void EventLoop::wakeUp() const {
    uint64_t one = 1;
    ssize_t n = ::write(wakeFd_, &one, sizeof one);
    if (n != sizeof one) {
        spdlog::error("EventLoop::wakeUp() reads {} bytes instead of 8", n);
    }
}

int EventLoop::createEventFd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        spdlog::error("Failed in eventfd");
        abort();
    }
    return evtfd;
}

void EventLoop::handleRead() const {
    uint64_t one = 1;
    ssize_t n = ::read(wakeFd_, &one, sizeof one);
    if (n != sizeof one) {
        spdlog::error("EventLoop::handleRead() reads {} bytes instead of 8", n);
    }
}

void EventLoop::updateInLoop(Channel* channel) {
    assert(isInLoopThread());
    assert(channel->ownerLoop() == this);
    poller.updateChannel(channel);
}

void EventLoop::removeInLoop(Channel *channel) {
    poller.removeChannel(channel);
}

void EventLoop::runAfter(uint64_t delay, const Channel::EventCallBack &callBack) {
    timerQueue.addTimer(callBack, delay, false);
}

void EventLoop::runAt(const Channel::EventCallBack &callBack) {
    timerQueue.addTimer(callBack, 1, false);
}
