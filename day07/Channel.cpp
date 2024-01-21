#include "Channel.h"

Channel::Channel(Epoll* ep, int fd, bool islisten) : ep_(ep), fd_(fd), islisten_(islisten) {

}

Channel::~Channel() {
// 在析构函数中，不要销毁 ep_, 也不能关闭 fd_ ，因为这两个东西不属于 Channel 类，Channel 只是需要它们，使用它们
}

int Channel::fd() const {
    return fd_;
}

void Channel::useet() {
    events_ = events_ | EPOLLET;
}

void Channel::enablereading() {
    events_ |= EPOLLIN;
    ep_->updatechannel(this);
}

void Channel::setinepoll() {
    inepoll_ = true;
}

void Channel::setrevents(uint32_t ev) {
    revents_ = ev;
}

bool Channel::inepoll() {
    return inepoll_;
}

uint32_t Channel::events() {
    return events_;
}

uint32_t Channel::revents() {
    return revents_;
}

void Channel::handleevent(Socket *servsock) {
    if (revents_ & EPOLLRDHUP) {                    // 对方已关闭，有些系统检测不到，可以使用 EPOLLIN recv() 返回 0
        printf("client(eventfd=%d) disconnected.\n", fd_);
        close(fd_);
    } else if (revents_ & (EPOLLIN | EPOLLPRI)) {   // 接收缓冲区中有数据可以读

        if (islisten_ == true) {   // 有客户端建立连接

            InetAddress clientaddr;
            Socket* clientsock = new Socket(servsock->accept(clientaddr));

            printf("accept client (fd=%d, ip=%s, port=%d) ok.\n", clientsock->fd(), clientaddr.ip(), clientaddr.port());

            Channel *clientchannel = new Channel(ep_, clientsock->fd(), false);
            clientchannel->useet();                  // 客户端采用边缘触发
            clientchannel->enablereading();

        } else {  // 连接的客户端 fd 有事件可读
            char buf[1024];
            while (true) {         // 由于采用非阻塞IO，一次性需要将 buf 中的数据全部读完
                bzero(&buf, sizeof(buf));
                ssize_t nread = read(fd_, buf, sizeof(buf));
                if (nread > 0) {   // 有数据，echo-server 全部写回给客户端
                    printf("recv(eventfd=%d): %s.\n", fd_, buf);
                    send(fd_, &buf, strlen(buf), 0);
                } else if (nread == -1 && errno == EINTR) {     // 读取数据的时候被信号中断，继续读取
                    continue;
                } else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {  // 非阻塞模式下，表示全部的数据已读取完毕
                    break;
                } else if (nread == 0) {     // 表示客户端已断开连接
                    printf("client(eventfd=%d) disconnected.\n", fd_);
                    close(fd_);
                    break;
                }
            }
        }
    } else if (revents_ & EPOLLOUT) {               // 有数据需要写，暂时不管
        // TODO
    } else {                                             // 其他事件，都视为错误
        printf("client(eventfd=%d) error.\n", fd_);
        close(fd_);
    }
}