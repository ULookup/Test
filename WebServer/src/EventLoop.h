#pragma once

#include "Poller.h"
#include "TimeWheel.h"
#include <thread>
#include <mutex>
#include <cassert>
#include <sys/eventfd.h>

namespace webserver::src
{

using Functor = std::function<void()>;

class EventLoop
{
public:
    EventLoop();
    /* brief: 判断将要执行的任务是否属于该EventLoop对应的线程，如果是就直接执行，如果不是就压入该EventLoop队列 */
    void RunInLoop(const Functor &cb);
    /* brief: 将需要该EventLoop执行的任务压入任务池 */
    void PushInLoop(const Functor &cb);
    /* brief: 用来断言当前线程是否是属于该EventLoop对应的线程 */
    void AssertInLoop() { assert(_thread_id == std::this_thread::get_id()); }
    /* brief: 用来判断当前线程是否是该EventLoop对应的线程 */
    bool IsInLoop() { return _thread_id == std::this_thread::get_id(); }

    // ================ 事件监控相关函数 ==================

    /* brief: 用来添加对传入的channel的事件监控 */ 
    void UpdateEvent(Channel *channel) { return _poller.UpdateEvents(channel); }
    /* brief: 用来移除对传入的channel的事件监控 */
    void RemoveEvent(Channel *channel) { return _poller.RemoveEvents(channel); }

    // ================ 计时器相关函数 ===================

    void AddTimer(uint64_t id, uint32_t delay, const TaskFunc &cb) { return _time_wheel.AddTimer(id, delay, cb); }
    void RefreshTimer(uint64_t id) { return _time_wheel.RefreshTimer(id); }
    void CancelTimer(uint64_t id) { return _time_wheel.CancelTimer(id); }
    bool HasTimer(uint64_t id) { return _time_wheel.HasTimer(id); }

    // ================ EventLoop 循环 ==================
    /* brief: EventLoop 的 Loop 循环所在 */
    void Start();
private:
    /* brief: 执行该EventLoop任务池的所有任务 */
    void RunAllTask();
    /* brief: 创建eventfd */
    static int CreateEventFd();
    /* brief: 读取EventFd */
    void ReadEventfd();
    /* brief: 通过EventFd唤醒 */
    void WakeUpEventFd();
private:
    std::thread::id _thread_id; // 该EventLoop所绑定的线程id
    int _eventfd;               // _eventfd 用于唤醒IO事件监控可能导致的阻塞
    std::unique_ptr<Channel> _event_channel;    // 为eventfd封装的channel
    Poller _poller;             // 执行所有channel的事件监控
    TimeWheel _time_wheel;      

    std::vector<Functor> _tasks; // 任务池
    std::mutex _mutex;
};

}

