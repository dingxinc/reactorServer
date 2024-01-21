#ifndef __INETADDRESS_H__
#define __INETADDRESS_H__

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

class InetAddress {
private:
    struct sockaddr_in addr_;

public:
    InetAddress();
    InetAddress(const std::string& ip, uint16_t port);          // 如果是服务器监听的 fd ，用这个构造函数
    InetAddress(struct sockaddr_in addr);                       // 如果是客户端连接上来，用这个构造函数
    ~InetAddress();

    const char* ip() const;    // 返回字符串表示的地址
    uint16_t port() const;     // 返回端口
    const sockaddr* addr() const;  // 返回 addr_ 成员的地址
    void setaddr(sockaddr_in clientaddr);
};


#endif /* __INETADDRESS_H__ */