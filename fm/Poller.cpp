//
// Created by Administrator on 2022/4/23.
//

#include "Poller.h"

Poller::Poller() {
    epfd = epoll_create(10);
}

int Poller::poll(int TimeOutMs, Poller::ChannelList *activeChannelList) {
    if ((int) pollfds.size() == 0)return 0;
    int nfd = epoll_wait(epfd, pollfds.data(), (int) pollfds.size(), TimeOutMs);
    if (nfd > 0) {
        for (int i = 0; i < nfd; ++i) {
            auto &channel = channelList[pollfds[i].data.fd];
            channel->setEvents(pollfds[i].events); // 为当前channel设置事件
            (*activeChannelList)[pollfds[i].data.fd] = channel;  // 处理事件
        }
    } else if (nfd == 0) {
        spdlog::info("Poller: nothing to do");
    } else {
        spdlog::error("Poller: poll() pollfds: {}" ,(int) pollfds.size());
    }

    return nfd;

}

void Poller::updateChannel(Channel *channel) {
    if (channel->getIndex() < 0) // index 是 chhannel在 pollfds 的位置
    {
        // a new one
        struct epoll_event ev{};
        ev.data.fd = channel->fd();
        ev.events = channel->getEvents();
        channelList[ev.data.fd] = channel;
        epoll_ctl(epfd, EPOLL_CTL_ADD, ev.data.fd, &ev);

        channel->setIndex((int) pollfds.size());
        pollfds.emplace_back(ev);
//        spdlog::info("new channel fd:{}",channel->fd());

    } else {
        // update
        auto ev = pollfds[channel->getIndex()];
        ev.data.fd = channel->fd();
        ev.events = channel->getEvents();

        channelList[ev.data.fd] = channel;
        if (ev.events == 0) {
            // 没有事件感兴趣，则屏蔽
            ev.data.fd = ev.data.fd - 1;
        } else {
            epoll_ctl(epfd, EPOLL_CTL_MOD, ev.data.fd, &ev);
        }
//        spdlog::info("update channel fd:{}" , channel->fd() );

    }

}

void Poller::removeChannel(Channel *channel) {
    if (channel->getIndex() >= 0) {
        auto ev = pollfds[channel->getIndex()];
        epoll_ctl(epfd, EPOLL_CTL_DEL, ev.data.fd, &ev);
        channelList.erase(channel->fd());
        channel->setIndex(-1);
    }
}
