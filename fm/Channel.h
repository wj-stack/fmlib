//
// Created by Administrator on 2022/4/23.
//

#ifndef FM_CHANNEL_H
#define FM_CHANNEL_H


#include <functional>
#include <sys/epoll.h>

#include "noncopyable.h"

class EventLoop;
class Channel : public noncopyable{
public:
    typedef std::function<void()> EventCallBack;
    Channel(EventLoop*,int);

    void setReadCallBack(const EventCallBack &callBack);

    void setWriteCallBack(const EventCallBack &callBack);

    void setErrorCallBack(const EventCallBack &callBack);

    void enableReading();

    void enableWriting();

    void disableWriting();

    void disableAll();

    void setEvents(int event); // 设置获取到的事件

    int getEvents() const { return events_; } //感兴趣的事件

    void handle();

    int fd() const;

    int getIndex() const { return index_; }

    void setIndex(int index);

    EventLoop* ownerLoop() const { return loop_; }

    void remove();

    bool isWriting() const { return revents_ & EVENT_WRITE; }

private:
    EventLoop* loop_;
    int fd_;  // 负责的fd
    int events_{}; // 感兴趣的事件
    int revents_ {}; // 当前的事件
    int index_ = -1;
    EventCallBack readCallBack;
    EventCallBack writeCallBack;
    EventCallBack errorCallBack;

    const int EVENT_NONE = 0;
    const int EVENT_READ = EPOLLIN | EPOLLPRI;
    const int EVENT_WRITE = EPOLLOUT;
    const int EVENT_ERR = EPOLLERR;
    const int EVENT_CLOSE = EPOLLRDHUP;

    void update();

};


#endif //FM_CHANNEL_H
