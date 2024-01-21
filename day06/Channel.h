#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include "Epoll.h"

class Epoll;

class Channel {
private:
    int fd_ = -1;                       // Channel 拥有的 fd, Channel 和 fd 是一对一的关系
    Epoll *ep_ = nullptr;               // Channel 对应的红黑树，Channel 与 Epoll 是多对一的关系，一个 Channel 只对应一个 Epoll
    bool inepoll_ = false;              // Channel 是否已添加到 epoll 树上，如果未添加，调用 epoll_ctl() 的时候用 EPOLL_CTL_ADD, 否则用 EPOLL_CTL_MOD
    uint32_t events_ = 0;               // fd_ 需要监视的事件，listenfd 和 clientfd 需要监视 EPOLLIN， clientfd 还可能需要监视 EPOLLOUT
    uint32_t revents_ = 0;              // fd_ 已发生的事件

public:
    Channel(Epoll* ep, int fd);
    ~Channel();

    int fd() const;                  // 返回 fd_ 成员
    void useet();                    // 采用边缘触发
    void enablereading();            // 让 epoll_wait() 监视 fd_ 的读事件
    void setinepoll();               // 把 inepoll_ 设置为 true
    void setrevents(uint32_t ev);    // 设置 revents_ 成员的值为参数 ev
    bool inepoll();                  // 返回 inepoll_ 成员
    uint32_t events();               // 返回 events_ 成员
    uint32_t revents();              // 返回 revents_ 成员
};


#endif /* __CHANNEL_H__ */