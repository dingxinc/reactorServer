#include <sys/fcntl.h>
#include <netinet/tcp.h>  // TCP_NODELAY 需要包含这个头文件
#include "InetAddress.h"
#include "Socket.h"
#include "Epoll.h"
#include "Channel.h"
#include "EventLoop.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("usage: ./server ip port\n");
        printf("example: ./server 172.19.163.2 9999\n");
        exit(-1);
    }

    Socket servsock(createnonblocking());
    InetAddress servaddr(argv[1], atoi(argv[2]));
    servsock.setreuseaddr(true);
    servsock.setreuseport(true);
    servsock.settcpnodelay(true);
    servsock.setkeepalive(true);
    servsock.bind(servaddr);
    servsock.listen();

    // Epoll ep;
    EventLoop loop;
    Channel *servchannel = new Channel(loop.ep(), servsock.fd());
    servchannel->setreadcallback(std::bind(&Channel::newconnection, servchannel, &servsock));  // 指定回调函数并绑定
    servchannel->enablereading();    // 启用 channel 的读事件

    loop.run();           // 运行事件循环

    /**
    while (true) {                  // 事件循环
        std::vector<Channel*> channels = ep.loop();    // 等待监视的 fd 有事件发生
        
        for (auto& ch : channels) {
            ch->handleevent();
        }
    } **/
    return 0;
}