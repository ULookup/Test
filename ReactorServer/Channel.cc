#include "Channel.hpp"
#include "EventLoop.hpp"

Channel::Channel(EventLoop *loop, int fd) : _fd(fd), _events(0), _revents(0), _loop(loop) {}
void Channel::SetFd(int fd) { _fd = fd; }
void Channel::SetRevents(uint32_t events) { _revents = events;}
void Channel::SetReadCallback(const EventCallback &cb) { _read_callback = cb; }
void Channel::SetWriteCallback(const EventCallback &cb) { _write_callback = cb; }
void Channel::SetErrorCallback(const EventCallback &cb) { _error_callback = cb; }
void Channel::SetCloseCallback(const EventCallback &cb) { _close_callback = cb; }
void Channel::SetEventCallback(const EventCallback &cb) { _event_callback = cb; }

/* brief: 获取想要监控的事件 */
uint32_t Channel::GetEvents() { return _events; }
uint32_t Channel::GetFd() { return _fd; }
/* brief: 当前是否监控可读 */
bool Channel::ReadAble() { return (_events & EPOLLIN); }
/* brief: 当前是否监控可写 */
bool Channel::WriteAble() { return (_events & EPOLLOUT); }
/* brief: 启动读事件监控 */
void Channel::EnableRead() { _events |= EPOLLIN; Update(); }
/* brief: 启动写事件监控 */
void Channel::EnableWrite() { _events |= EPOLLOUT; Update(); }
/* brief: 关闭读事件监控 */
void Channel::DisableRead() { _events &= ~EPOLLIN; Update(); } 
/* brief: 关闭写事件监控 */
void Channel::DisableWrite() { _events &= ~EPOLLOUT; Update(); }
/* brief: 关闭所有事件监控 */
void Channel::DisableAll() { _events = 0; Update(); }
/* brief: 移除监控*/
void Channel::Remove() { return _loop->RemoveEvent(this); }

void Channel::Update() { return _loop->UpdateEvent(this); }
/* brief: 连接触发事件就调用这个函数*/
void Channel::HandlerEvent() {
    if ((_revents & EPOLLIN) || (_revents & EPOLLRDHUP) || (_revents & EPOLLPRI)) {
        if (_read_callback) _read_callback();
        //LOG_DEBUG << "_READ_CALLBACK()";
    }
    if (_revents & EPOLLOUT) {
        if (_write_callback) _write_callback();
        //LOG_DEBUG << "_WRITE_CALLBACK()";

    } else if (_revents & EPOLLERR) {
        if (_error_callback) _error_callback();
        //LOG_DEBUG << "_ERROR_CALLBACK()";
    } else if (_revents & EPOLLHUP) {
        if (_close_callback) _close_callback();
        //LOG_DEBUG << "_CLOSE_CALLBACK()";
    }
    if (_event_callback) _event_callback();
        //LOG_DEBUG << "_EVENT_CALLBACK()";
}