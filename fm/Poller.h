//
// Created by Administrator on 2022/4/23.
//

#ifndef FM_POLLER_H
#define FM_POLLER_H

#include <map>
#include "noncopyable.h"
#include "spdlog/spdlog.h"
#include "Channel.h"

class Poller : public noncopyable {
public:

    using ChannelList = std::map<int, Channel *>; // 一个fd对应着一个channel

    Poller();

    int poll(int TimeOutMs, ChannelList *activeChannelList);

    void updateChannel(Channel *channel);

    void removeChannel(Channel *channel);

private:
    ChannelList channelList;
    std::vector<struct epoll_event> pollfds; // fd的集合
    int epfd = -1;

};


#endif //FM_POLLER_H
