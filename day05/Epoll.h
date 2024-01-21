#ifndef __EPOLL_H__
#define __EPOLL_H__

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include <sys/epoll.h>
#include <vector>
#include <unistd.h>

class Epoll {
private:
    static const int MAXEVENTS = 100;     // epoll_wait() 返回事件数组的大小
    int epollfd_ = -1;                     // epoll 句柄，在构造函数中创建
    epoll_event events_[MAXEVENTS];       // 存放 epoll_wait() 返回事件的数据

public:
    Epoll();
    ~Epoll();

    void addfd(int fd, uint32_t op);       // 把 fd 和它需要监视的事件添加到红黑树中
    std::vector<epoll_event> loop(int timeout = -1);  // 运行等待 epoll_wait(), 等待事件发生，已发生的事件用 vector 返回
};


#endif /* __EPOLL_H__ */