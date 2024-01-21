#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"

class TcpServer {
private:
    EventLoop loop_;            // 一个 TcpServer 可以有多个事件循环，现在是单线程，暂时只用一个事件循环

public:
    TcpServer(const std::string& ip, const uint16_t port);
    ~TcpServer();

    void start();     // 运行事件循环
};


#endif /* __TCP_SERVER_H__ */