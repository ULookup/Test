#pragma once

#include "Logger/Logger.h"
#include <cstdint>
#include <functional>
#include <sys/epoll.h>

using EventCallback = std::function<void()>;

class EventLoop;

class Channel
{
public:
    Channel(EventLoop *loop, int fd);
    void SetRevents(uint32_t events);
    void SetReadCallback(const EventCallback &cb);
    void SetWriteCallback(const EventCallback &cb);
    void SetErrorCallback(const EventCallback &cb);
    void SetCloseCallback(const EventCallback &cb);
    void SetEventCallback(const EventCallback &cb);

    /* brief: 获取想要监控的事件 */
    uint32_t GetEvents();
    uint32_t GetFd();
    /* brief: 当前是否监控可读 */
    bool ReadAble();
    /* brief: 当前是否监控可写 */
    bool WriteAble();
    /* brief: 启动读事件监控 */
    void EnableRead();
    /* brief: 启动写事件监控 */
    void EnableWrite();
    /* brief: 关闭读事件监控 */
    void DisableRead();
    /* brief: 关闭写事件监控 */
    void DisableWrite();
    /* brief: 关闭所有事件监控 */
    void DisableAll();
    /* brief: 移除监控*/
    void Remove();

    void Update();
    /* brief: 连接触发事件就调用这个函数*/
    void HandlerEvent();
private:
    int _fd;
    EventLoop* _loop;
    uint32_t _events;
    uint32_t _revents;
    EventCallback _read_callback;
    EventCallback _write_callback;
    EventCallback _error_callback;
    EventCallback _close_callback;
    EventCallback _event_callback;
};