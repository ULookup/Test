#pragma once

#include <cstdint>
#include <functional>
#include <sys/epoll.h>

using EventCallback = std::function<void()>;

class Channel
{
public:
    Channel(int fd) : _fd(fd) {}
    void SetRevents(uint32_t events) { _revents = events;}
    void SetReadCallback(const EventCallback &cb) { _read_callback = cb; }
    void SetWriteCallback(const EventCallback &cb) { _write_callback = cb; }
    void SetErrorCallback(const EventCallback &cb) { _error_callback = cb; }
    void SetCloseCallback(const EventCallback &cb) { _close_callback = cb; }
    void SetEventCallback(const EventCallback &cb) { _event_callback = cb; }

    /* brief: 获取想要监控的事件 */
    uint32_t GetEvents() { return _events; }
    uint32_t GetFd() { return _fd; }
    /* brief: 当前是否监控可读 */
    bool ReadAble() { return (_events & EPOLLIN); }
    /* brief: 当前是否监控可写 */
    bool WriteAble() { return (_events & EPOLLOUT); }
    /* brief: 启动读事件监控 */
    void EnableRead() { _events |= EPOLLIN; }
    /* brief: 启动写事件监控 */
    void EnableWrite() { _events |= EPOLLOUT; }
    /* brief: 关闭读事件监控 */
    void DisableRead() { _events &= ~EPOLLIN; } 
    /* brief: 关闭写事件监控 */
    void DisableWrite() { _events &= ~EPOLLOUT; }
    /* brief: 关闭所有事件监控 */
    void DisableAll() { _events = 0; }
    /* brief: 移除监控*/
    void Remove();
    /* brief: 连接触发事件就调用这个函数*/
    void HandlerEvent() {
        if ((_revents & EPOLLIN) || (_revents & EPOLLRDHUP) || (_revents & EPOLLPRI)) {
            if (_read_callback) _read_callback();

            if (_event_callback) _event_callback();
        }

        if (_revents & EPOLLOUT) {
            if (_write_callback) _write_callback();

            if (_event_callback) _event_callback();
        } else if (_revents & EPOLLERR) {
            if (_event_callback) _event_callback();

            if (_error_callback) _error_callback();
        } else if (_revents & EPOLLHUP) {
            if (_event_callback) _event_callback();

            if (_close_callback) _close_callback();
        }
        
    }
private:
    int _fd;
    uint32_t _events;
    uint32_t _revents;
    EventCallback _read_callback;
    EventCallback _write_callback;
    EventCallback _error_callback;
    EventCallback _close_callback;
    EventCallback _event_callback;
};