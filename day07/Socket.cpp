#include "Socket.h"

int createnonblocking() {
    int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if (listenfd < 0) {
        printf("%s:%s:%d listen socket create error: %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        exit(-1);
    }
    return listenfd;
}

Socket::Socket(int fd) : fd_(fd) {

}

Socket::~Socket() {
    ::close(fd_);
}

int Socket::fd() const {
    return fd_;
}

void Socket::setreuseaddr(bool no) {
    int optval = no ? 1 : 0;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

void Socket::setreuseport(bool no) {
    int optval = no ? 1 : 0;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}

void Socket::settcpnodelay(bool no) {
    int optval = no ? 1 : 0;
    ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

void Socket::setkeepalive(bool no) {
    int optval = no ? 1 : 0;
    ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}

void Socket::bind(const InetAddress& servaddr) {
    if (::bind(fd_, servaddr.addr(), sizeof(sockaddr)) < 0) {
        perror("bind() error"); close(fd_); exit(-1);
    }
}

void Socket::listen(const int n) {
    if (::listen(fd_, n) != 0) {
        perror("listen() error"); close(fd_); exit(-1);
    }
}

int Socket::accept(InetAddress& clientaddr) {
    sockaddr_in peeraddr;
    socklen_t len = sizeof(peeraddr);
    int clientfd = accept4(fd_, (sockaddr*)&peeraddr, &len, SOCK_NONBLOCK);

    clientaddr.setaddr(peeraddr);   // 客户端的地址和协议
    return clientfd;
}