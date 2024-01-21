#include "TcpServer.h"

TcpServer::TcpServer(const std::string& ip, const uint16_t port) {
    Socket *servsock = new Socket(createnonblocking());
    InetAddress servaddr(ip, port);
    servsock->setreuseaddr(true);
    servsock->setreuseport(true);
    servsock->settcpnodelay(true);
    servsock->setkeepalive(true);
    servsock->bind(servaddr);
    servsock->listen();

    Channel *servchannel = new Channel(loop_.ep(), servsock->fd());
    servchannel->setreadcallback(std::bind(&Channel::newconnection, servchannel, servsock));  // 指定回调函数并绑定
    servchannel->enablereading();    // 启用 channel 的读事件
}

TcpServer::~TcpServer() {

}

void TcpServer::start() {
    loop_.run();
}