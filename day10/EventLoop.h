#ifndef __EVENT_LOOP_H__
#define __EVENT_LOOP_H__

#include "Epoll.h"

class EventLoop {
private:
    Epoll* ep_;          // 每个事件循环只有一个 Epoll

public:
    EventLoop();         // 在构造函数中创建 Epoll 对象 ep_
    ~EventLoop();        // 在析构函数中销毁 ep_

    void run();          // 运行事件循环
    Epoll *ep();         // 返回 ep_ 成员函数
};

#endif /* __EVENT_LOOP_H__ */