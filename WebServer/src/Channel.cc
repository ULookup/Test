#include "Channel.h"
#include <sys/epoll.h>

namespace webserver::src
{

Channel::Channel(EventLoop *loop, int fd) : _fd(fd), _loop(loop), _events(0), _revents(0) {}

void Channel::SetFd(int fd) { _fd = fd; }
void Channel::SetRevent(uint32_t events) { _revents = events; }
void Channel::SetReadCallback(const EventCallback &cb) { _read_callback = cb; }
void Channel::SetWriteCallback(const EventCallback &cb) { _write_callback = cb; }
void Channel::SetErrorCallback(const EventCallback &cb) { _error_callback = cb; }
void Channel::SetCloseCallback(const EventCallback &cb) { _close_callback = cb; }
void Channel::SetEventCallback(const EventCallback &cb) { _event_callback = cb; }

int Channel::GetFd() { return _fd; }
uint32_t Channel::GetEvents() { return _events; }

bool Channel::ReadAble() { return (_events & EPOLLIN); }
bool Channel::WritAble() { return (_events & EPOLLOUT); }

void Channel::EnableRead() { _events |= EPOLLIN; Update(); }
void Channel::EnableWrite() { _events |= EPOLLOUT; Update(); }
void Channel::DisableRead() { _events &= ~EPOLLIN; Update(); }
void Channel::DisableWrite() { _events &= ~EPOLLOUT; Update(); }
void Channel::DisableAll() { _events = 0; Update(); }

void Channel::Remove() { return _loop->RemoveEvent(this); }
void Channel::Update() { return _loop->UpdateEvent(this); }

void Channel::HandlerEvent() {
    if((_revents & EPOLLIN) || (_revents & EPOLLRDHUP) || (_revents & EPOLLPRI)) {
        if(_read_callback) _read_callback();
    }

    if((_revents & EPOLLOUT)) {
        /* epoll 监控到写事件就绪 */
        if(_write_callback) _write_callback();
    }
    else if(_revents & EPOLLERR) {
        /* epoll 监控出错了 */
        if(_error_callback) _error_callback();
    }
    else if(_revents & EPOLLHUP) {
        /* epoll 监控到对端挂断了 */
        if(_close_callback) _close_callback();
    }
    
    if(_event_callback) _event_callback();
}
}