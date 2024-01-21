#include <sys/fcntl.h>
#include <netinet/tcp.h>  // TCP_NODELAY 需要包含这个头文件
#include "InetAddress.h"
#include "Socket.h"
#include "Epoll.h"

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

    /**
    // 创建 epoll 句柄（红黑树）
    int epollfd = epoll_create(1);

    // 为服务端的 listenfd 准备读事件
    struct epoll_event ev;          // 声明事件的数据结构
    ev.data.fd = servsock.fd();          // 指定事件的自定义结构，会随 epoll_wait() 返回值一起返回
    ev.events = EPOLLIN;            // 采用水平触发，监听读事件

    epoll_ctl(epollfd, EPOLL_CTL_ADD, servsock.fd(), &ev);   // 将 listenfd 挂到红黑树上，委托 epoll 监听

    struct epoll_event evs[10];            // 存放 epoll_wait() 返回事件的数组 **/

    Epoll ep;
    ep.addfd(servsock.fd(), EPOLLIN);       // 监听 listenfd 的读事件
    std::vector<epoll_event> evs;    // 存放 epoll_wait 返回的事件

    while (true) {                  // 事件循环
        /***
        int infds = epoll_wait(epollfd, evs, 10, -1);   // 等待监听的 fd 有事件发生
        if (infds < 0) {   // 返回失败
            perror("epoll wait failed."); break;
        }

        if (infds == 0) {  // 超时
            perror("epoll timeout."); continue;
        }  **/

        // infds > 0 , 有事件返回
        //for (int i = 0; i < infds; ++i) {
        evs = ep.loop();    // 等待监视的 fd 有事件发生
        
        for (auto& ev : evs) {
            if (ev.events & EPOLLRDHUP) {                    // 对方已关闭，有些系统检测不到，可以使用 EPOLLIN recv() 返回 0
                printf("client(eventfd=%d) disconnected.\n", ev.data.fd);
                close(ev.data.fd);
            } else if (ev.events & (EPOLLIN | EPOLLPRI)) {   // 接收缓冲区中有数据可以读

                if (ev.data.fd == servsock.fd()) {   // 有客户端建立连接
                    /** struct sockaddr_in peeraddr;
                    socklen_t len = sizeof(peeraddr);
                    int clientfd = accept4(listenfd, (struct sockaddr*)&peeraddr, &len, SOCK_NONBLOCK); **/

                    InetAddress clientaddr;
                    Socket* clientsock = new Socket(servsock.accept(clientaddr));

                    printf("accept client (fd=%d, ip=%s, port=%d) ok.\n", clientsock->fd(), clientaddr.ip(), clientaddr.port());

                    // 为新客户端连接准备读事件，并加入到 epoll 中
                    /**ev.data.fd = clientsock->fd();
                    ev.events = EPOLLIN | EPOLLET;        // 边缘触发
                    epoll_ctl(epollfd, EPOLL_CTL_ADD, clientsock->fd(), &ev); **/
                    ep.addfd(clientsock->fd(), EPOLLIN | EPOLLET);   // 客户端连上来的 fd 采用边缘触发

                } else {  // 连接的客户端 fd 有事件可读
                    char buf[1024];
                    while (true) {         // 由于采用非阻塞IO，一次性需要将 buf 中的数据全部读完
                        bzero(&buf, sizeof(buf));
                        ssize_t nread = read(ev.data.fd, buf, sizeof(buf));
                        if (nread > 0) {   // 有数据，echo-server 全部写回给客户端
                            printf("recv(eventfd=%d): %s.\n", ev.data.fd, buf);
                            send(ev.data.fd, &buf, strlen(buf), 0);
                        } else if (nread == -1 && errno == EINTR) {     // 读取数据的时候被信号中断，继续读取
                            continue;
                        } else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {  // 非阻塞模式下，表示全部的数据已读取完毕
                            break;
                        } else if (nread == 0) {     // 表示客户端已断开连接
                            printf("client(eventfd=%d) disconnected.\n", ev.data.fd);
                            close(ev.data.fd);
                            break;
                        }
                    }
                }
            } else if (ev.events & EPOLLOUT) {               // 有数据需要写，暂时不管
                // TODO
            } else {                                             // 其他事件，都视为错误
                printf("client(eventfd=%d) error.\n", ev.data.fd);
                close(ev.data.fd);
            }
        }
    }
    return 0;
}