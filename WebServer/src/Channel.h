#pragma once

#include <cstdint>
#include <functional>

// author: Haoyang Yang
// filename: Channel.h

namespace webserver::src 
{

using EventCallback = std::function<void()>;

class EventLoop;

class Channel
{
public:
    Channel(EventLoop *loop, int fd);
    ~Channel() = default;
    // ======== Usage: 对 Channel 进行配置的接口 ======== //
    /* brief: SetFd 用于设置 Channel 对应的套接字文件描述符 */
    void SetFd(int fd);
    /* brief: SetRevent ...*/
    void SetRevent(uint32_t events);
    /* brief: 以下接口都是用于设置 epoll 事件就绪后触发的回调函数 */
    void SetReadCallback(const EventCallback &cb);
    void SetWriteCallback(const EventCallback &cb);
    void SetErrorCallback(const EventCallback &cb);
    void SetCloseCallback(const EventCallback &cb);
    void SetEventCallback(const EventCallback &cb);
    // ================================================== //
    /* brief: 以下接口都是用于获取 Channel 有关信息的接口 */
    int GetFd();
    uint32_t GetEvents();

    /* brief: 用于判断是否开启了读/写监控 */
    bool ReadAble();
    bool WritAble();

    /* brief: 用于对 epoll 关心的事件进行设置。底层动作就是对“关心”红黑树进行 CRUD （用 epoll_ctl） */
    void EnableRead();
    void EnableWrite();
    void DisableRead();
    void DisableWrite();
    void DisableAll();

    /* brief： epoll_ctl 动作的直接接口，通向该 子Reactor 的 poll模块 */
    void Remove();
    void Update();

    /* brief: 在 poll 对应关心的事件就绪后，判断是哪个事件，并分配执行对应事件回调 */
    void HandlerEvent();
private:
    int _fd;
    EventLoop *_loop;
    uint32_t _events;
    uint32_t _revents;

    EventCallback _read_callback;
    EventCallback _write_callback;
    EventCallback _error_callback;
    EventCallback _close_callback;
    EventCallback _event_callback;
};


}