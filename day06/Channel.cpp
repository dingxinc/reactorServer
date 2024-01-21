#include "Channel.h"

Channel::Channel(Epoll* ep, int fd) : ep_(ep), fd_(fd) {

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