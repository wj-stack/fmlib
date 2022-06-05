//
// Created by wyatt on 2022/4/20.
//

#ifndef SOCKETFRAMEWORK_TIMER_H
#define SOCKETFRAMEWORK_TIMER_H

#include <functional>
#include <cinttypes>

class Timer {
public:
    Timer(const std::function<void()> &cb, uint64_t delay, bool recycle);

    void reset();

    void run();

    uint64_t getTimeStamp() const { return timeStamp; }

    bool isRecycle() const { return recycle; }

    int getId() const { return id; }

private:
    static int sid;
    int id;
    std::function<void()> cb;
    uint64_t delay;
    uint64_t timeStamp;
    bool recycle;
};


#endif //SOCKETFRAMEWORK_TIMER_H
