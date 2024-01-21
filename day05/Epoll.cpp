#include "Epoll.h"

Epoll::Epoll() {
    if ((epollfd_ = epoll_create(1)) == -1) {
        printf("epoll_create() failed(%d).\n", errno); exit(-1);
    }
}

Epoll::~Epoll() {
    close(epollfd_);
}

void Epoll::addfd(int fd, uint32_t op) {
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = op;

    if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        printf("epoll_ctl() failed(%d).\n", errno);
    }
}

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
}