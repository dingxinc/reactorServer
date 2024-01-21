#include "Epoll.h"

Epoll::Epoll() {
    if ((epollfd_ = epoll_create(1)) == -1) {
        printf("epoll_create() failed(%d).\n", errno); exit(-1);
    }
}

Epoll::~Epoll() {
    close(epollfd_);
}

/**
void Epoll::addfd(int fd, uint32_t op) {
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = op;

    if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        printf("epoll_ctl() failed(%d).\n", errno);
    }
} **/

void Epoll::updatechannel(Channel* ch) {
    epoll_event ev;         // 声明事件的数据结构
    ev.data.ptr = ch;       // 指定 channel
    ev.events = ch->events();// 指定事件

    if (ch->inepoll()) {     // 如果 channel 已经在树上了
        if (epoll_ctl(epollfd_, EPOLL_CTL_MOD, ch->fd(), &ev) == -1) {
            perror("epoll_ctl() failed.\n"); exit(-1);
        }
    } else {                 // 如果 channel 不在树上
        if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, ch->fd(), &ev) == -1) {
            perror("epoll_ctl() failed.\n"); exit(-1);
        }
        ch->setinepoll();             // 把 channel 的 inepoll_ 成员设置为 true
    }
}

/**
std::vector<epoll_event> Epoll::loop(int timeout) {
    std::vector<epoll_event> evs;    // 存放 epoll_wait 返回的事件

    bzero(events_, sizeof(events_));
    int infds = epoll_wait(epollfd_, events_, MAXEVENTS, timeout);

    // 返回失败
    if (infds < 0) {
        perror("epoll_wait() failed\n"); exit(-1);
    }

    // 超时
    if (infds == 0) {
        printf("epoll_wait() timeout.\n"); return evs;
    }

    // 如果 infds > 0 表示有事件发生的 fd 的数量
    for (int i = 0; i < infds; ++i) {
        evs.push_back(events_[i]);
    }

    return evs;
}  **/

std::vector<Channel*> Epoll::loop(int timeout) {
    std::vector<Channel*> channels;    // 存放 epoll_wait 返回的事件

    bzero(events_, sizeof(events_));
    int infds = epoll_wait(epollfd_, events_, MAXEVENTS, timeout);

    // 返回失败
    if (infds < 0) {
        perror("epoll_wait() failed\n"); exit(-1);
    }

    // 超时
    if (infds == 0) {
        printf("epoll_wait() timeout.\n"); return channels;
    }

    // 如果 infds > 0 表示有事件发生的 fd 的数量
    for (int i = 0; i < infds; ++i) {
        Channel *ch = (Channel*)events_[i].data.ptr;       // 取出已发生事件的 channel
        ch->setrevents(events_[i].events);                 // 设置 channel 的 revents_ 成员
        channels.push_back(ch);
    }

    return channels;
}
