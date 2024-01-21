#include <sys/fcntl.h>
#include <netinet/tcp.h>  // TCP_NODELAY 需要包含这个头文件
#include "InetAddress.h"
#include "Socket.h"
#include "Epoll.h"
#include "Channel.h"

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

    Epoll ep;
    // ep.addfd(servsock.fd(), EPOLLIN);       // 监听 listenfd 的读事件
    Channel *servchannel = new Channel(&ep, servsock.fd());
    servchannel->enablereading();    // 启用 channel 的读事件

    while (true) {                  // 事件循环
        std::vector<Channel*> channels = ep.loop();    // 等待监视的 fd 有事件发生
        
        for (auto& ch : channels) {
            if (ch->revents() & EPOLLRDHUP) {                    // 对方已关闭，有些系统检测不到，可以使用 EPOLLIN recv() 返回 0
                printf("client(eventfd=%d) disconnected.\n", ch->fd());
                close(ch->fd());
            } else if (ch->revents() & (EPOLLIN | EPOLLPRI)) {   // 接收缓冲区中有数据可以读

                if (ch == servchannel) {   // 有客户端建立连接

                    InetAddress clientaddr;
                    Socket* clientsock = new Socket(servsock.accept(clientaddr));

                    printf("accept client (fd=%d, ip=%s, port=%d) ok.\n", clientsock->fd(), clientaddr.ip(), clientaddr.port());

                    // ep.addfd(clientsock->fd(), EPOLLIN | EPOLLET);   // 客户端连上来的 fd 采用边缘触发
                    Channel *clientchannel = new Channel(&ep, clientsock->fd());
                    clientchannel->useet();                  // 客户端采用边缘触发
                    clientchannel->enablereading();

                } else {  // 连接的客户端 fd 有事件可读
                    char buf[1024];
                    while (true) {         // 由于采用非阻塞IO，一次性需要将 buf 中的数据全部读完
                        bzero(&buf, sizeof(buf));
                        ssize_t nread = read(ch->fd(), buf, sizeof(buf));
                        if (nread > 0) {   // 有数据，echo-server 全部写回给客户端
                            printf("recv(eventfd=%d): %s.\n", ch->fd(), buf);
                            send(ch->fd(), &buf, strlen(buf), 0);
                        } else if (nread == -1 && errno == EINTR) {     // 读取数据的时候被信号中断，继续读取
                            continue;
                        } else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {  // 非阻塞模式下，表示全部的数据已读取完毕
                            break;
                        } else if (nread == 0) {     // 表示客户端已断开连接
                            printf("client(eventfd=%d) disconnected.\n", ch->fd());
                            close(ch->fd());
                            break;
                        }
                    }
                }
            } else if (ch->revents() & EPOLLOUT) {               // 有数据需要写，暂时不管
                // TODO
            } else {                                             // 其他事件，都视为错误
                printf("client(eventfd=%d) error.\n", ch->fd());
                close(ch->fd());
            }
        }
    }
    return 0;
}