//
// Created by Administrator on 2022/4/23.
//

#include "Channel.h"
#include "EventLoop.h"

Channel::Channel(EventLoop *eventLoop, int fd) : loop_(eventLoop), fd_(fd) {

}

void Channel::setReadCallBack(const Channel::EventCallBack &callBack) {
    Channel::readCallBack = callBack;
}

void Channel::setWriteCallBack(const Channel::EventCallBack &callBack) {
    Channel::writeCallBack = callBack;
}

void Channel::setErrorCallBack(const Channel::EventCallBack &callBack) {
    Channel::errorCallBack = callBack;
}

void Channel::update() {
    loop_->updateChannel(this);
}

void Channel::enableReading() {
    events_ |= EVENT_READ;
    update();
}

void Channel::disableAll() {
    events_ = EVENT_NONE;
    update();
}

void Channel::enableWriting() {
    events_ |= EVENT_WRITE;
    update();
}

void Channel::disableWriting() {
    events_ &= ~EVENT_WRITE;
    update();
}

void Channel::setEvents(int event) {
    revents_ = event;
}

void Channel::handle() {
    if (revents_ & EVENT_ERR) {
        if (errorCallBack)errorCallBack();
    }
    if (revents_ & EVENT_READ) {
        if (readCallBack)readCallBack();
    }
    if (revents_ & EVENT_WRITE) {
        if (writeCallBack)writeCallBack();
    }
}

int Channel::fd() const {
    return fd_;
}

void Channel::remove() {
    loop_->removeChannel(this);
}

void Channel::setIndex(int index)  { index_ =  index; }
