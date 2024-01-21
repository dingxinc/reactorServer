#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "InetAddress.h"

// 创建一个非阻塞的 Socket
int createnonblocking();

class InetAddress;

class Socket {
private:
    const int fd_;          // Socket 持有的 fd_, 构造函数中传进来

public:
    Socket(int fd);
    ~Socket();              // 关闭 fd

    int fd() const;         // 返回 fd_ 成员
    void setreuseaddr(bool no);
    void setreuseport(bool no);
    void settcpnodelay(bool no);
    void setkeepalive(bool no);

    void bind(const InetAddress& servaddr);
    void listen(const int n = 128);
    int accept(InetAddress& clientaddr);
};


#endif /* __SOCKET_H__ */