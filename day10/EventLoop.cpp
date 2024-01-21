#include "EventLoop.h"

EventLoop::EventLoop() :ep_(new Epoll) {

}

EventLoop::~EventLoop() {
    delete ep_;
}

void EventLoop::run() {
    while (true) {                  // 事件循环
        std::vector<Channel*> channels = ep_->loop();    // 等待监视的 fd 有事件发生
            
        for (auto& ch : channels) {
            ch->handleevent();
        }
    }
}

Epoll* EventLoop::ep() {
    return ep_;
}