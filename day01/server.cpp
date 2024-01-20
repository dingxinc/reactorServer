#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>  // TCP_NODELAY 需要包含这个头文件

void setnonblocking(int fd) {   // 设置非阻塞
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("usage: ./server ip port\n");
        printf("example: ./server 172.19.163.2 9999\n");
        exit(-1);
    }

    // 创建用于监听的文件描述符
    int listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen < 0) {
        perror("soscket() error");
        return -1;   // 主程序中碰到 return 语句，会退出程序的执行
    }

    // 设置 listenfd 的属性
    int opt;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof(opt)));   // 必须的
    setsockopt(listenfd, SOL_SOCKET, TCP_NODELAY, &opt, static_cast<socklen_t>(sizeof(opt)));    // 必须的
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof(opt)));   // Reactor 中用处不大
    setsockopt(listenfd, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof(opt)));   // 建议自己实现心跳机制

    // 设置 fd 的非阻塞模式
    setnonblocking(listenfd);

    // 绑定、监听
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind() error"); return -1;
    }

    if (listen(listenfd, 128) != 0) {
        perror("listen() error"); return -1;
    }

    // 创建 epoll 句柄（红黑树）
    int epollfd = epoll_create(1);

    // 为服务端的 listenfd 准备读事件
    struct epoll_event ev;          // 声明事件的数据结构
    ev.data.fd = listenfd;          // 指定事件的自定义结构，会随 epoll_wait() 返回值一起返回
    ev.events = EPOLLIN;            // 采用水平触发，监听读事件

    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);   // 将 listenfd 挂到红黑树上，委托 epoll 监听

    struct epoll_event evs[10];            // 存放 epoll_wait() 返回事件的数组

    while (true) {                  // 事件循环
        int infds = epoll_wait(epollfd, evs, 10, -1);   // 等待监听的 fd 有事件发生
        if (infds < 0) {   // 返回失败
            perror("epoll wait failed."); break;
        }

        if (infds == 0) {  // 超时
            perror("epoll timeout."); continue;
        }

        // infds > 0 , 有事件返回
        for (int i = 0; i < infds; ++i) {
            if (evs[i].data.fd == listenfd) {   // 有客户端建立连接
                struct sockaddr_in clientaddr;
                socklen_t len = sizeof(clientaddr);
                int clientfd = accept(listenfd, (struct sockaddr*)&clientaddr, &len);
                setnonblocking(clientfd);       // 为客户端 fd 设置非阻塞模式

                printf("accept client (fd=%d, ip=%s, port=%d) ok.\n", clientfd, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

                // 为新客户端连接准备读事件，并加入到 epoll 中
                ev.data.fd = clientfd;
                ev.events = EPOLLIN | EPOLLET;        // 边缘触发
                epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &ev);
            } else {        // 如果客户端连接的 fd 有事件
                if (evs[i].events & EPOLLRDHUP) {                    // 对方已关闭，有些系统检测不到，可以使用 EPOLLIN recv() 返回 0
                    printf("client(eventfd=%d) disconnected.\n", evs[i].data.fd);
                    close(evs[i].data.fd);
                } else if (evs[i].events & (EPOLLIN | EPOLLPRI)) {   // 接收缓冲区中有数据可以读
                    char buf[1024];
                    while (true) {         // 由于采用非阻塞IO，一次性需要将 buf 中的数据全部读完
                        bzero(&buf, sizeof(buf));
                        ssize_t nread = read(evs[i].data.fd, buf, sizeof(buf));
                        if (nread > 0) {   // 有数据，echo-server 全部写回给客户端
                            printf("recv(eventfd=%d): %s.\n", evs[i].data.fd, buf);
                            send(evs[i].data.fd, &buf, strlen(buf), 0);
                        } else if (nread == -1 && errno == EINTR) {     // 读取数据的时候被信号中断，继续读取
                            continue;
                        } else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {  // 非阻塞模式下，表示全部的数据已读取完毕
                            break;
                        } else if (nread == 0) {     // 表示客户端已断开连接
                            printf("client(eventfd=%d) disconnected.\n", evs[i].data.fd);
                            close(evs[i].data.fd);
                            break;
                        }
                    }
                } else if (evs[i].events & EPOLLOUT) {               // 有数据需要写，暂时不管
                    // TODO
                } else {                                             // 其他事件，都视为错误
                    printf("client(eventfd=%d) error.\n", evs[i].data.fd);
                    close(evs[i].data.fd);
                }
            }
        }
    }
    return 0;
}