#pragma once

#include "Logger/Logger.h"
#include "TimeWheel.hpp"
#include "Poller.hpp"
#include <vector>
#include <sys/eventfd.h>
#include <mutex>
#include <thread>

using Functor = std::function<void()>;

class EventLoop 
{
public:
    EventLoop():_thread_id(std::this_thread::get_id()), 
                _eventfd(CreateEventFd()),
                _event_channel(std::make_unique<Channel>(this, _eventfd)),
                _time_wheel(this) {
        /* brief: 给 _eventfd 添加可读事件回调函数，读取 _eventfd 事件通知次数 */
        LOG_DEBUG << "即将给eventfd添加可读事件回调函数";
        _event_channel->SetReadCallback(std::bind(&EventLoop::ReadEventfd,this));
        /* brief: 启动 _eventfd 读事件监控 */
        LOG_DEBUG << "启动eventfd读事件监控";
        _event_channel->EnableRead();
    }
    /* brief: 判断将要执行的任务是否处于当前线程中，如果是则执行，不是则压入队列 */
    void RunInLoop(const Functor &cb) {
        if(IsInLoop()) {
            return cb();
        }
        return PushInLoop(cb);
    }
    /* brief: 将操作压入任务池 */
    void PushInLoop(const Functor &cb) {
        {
            std::unique_lock<std::mutex> _lock(_mutex);
            _tasks.push_back(cb);
        }
        // 唤醒可能因为没有事件就绪，而导致的epoll阻塞，给eventfd写入一个数据，触发可读事件
        WakeUpEventFd();
    }
    /* brief: 判断当前线程是否是对应 EventLoop 线程 */
    bool IsInLoop() {
        return _thread_id == std::this_thread::get_id();
    }
    /* brief: 添加对 channel 的事件监控 */
    void UpdateEvent(Channel *channel) { return _poller.UpdateEvents(channel); }
    /* brief: 移除对 channel 的事件监控 */
    void RemoveEvent(Channel *channel) { return _poller.RemoveEvents(channel); }

    void AddTimer(uint64_t id, uint32_t delay, const TaskFunc &cb) { return _time_wheel.AddTimer(id, delay, cb); }
    void RefreshTimer(uint64_t id) { return _time_wheel.RefreshTimer(id); }
    void CancelTimer(uint64_t id) { return _time_wheel.CancelTimer(id); }
    bool HasTimer(uint64_t id) { return _time_wheel.HasTimer(id); }
    /* brief: 三步走：事件监控->就绪事件处理->执行任务 */
    void Start() {
        // step1: 事件监控
        LOG_DEBUG << "开始监控事件";
        std::vector<Channel*> actives;
        _poller.Poll(actives);
        // step2: 就绪事件处理
        LOG_DEBUG << "开始就绪事件处理";
        for(auto &channel : actives) {
            channel->HandlerEvent();
        }
        // step3: 执行任务
        LOG_DEBUG << "开始执行任务";
        RunAllTask();
    }
private:
    void RunAllTask() {
        std::vector<Functor> functor;
        {
            std::unique_lock<std::mutex> _lock(_mutex);
            _tasks.swap(functor);
        }
        for(auto &f : functor) { f(); }

        return;
    }

    static int CreateEventFd() {
        int efd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
        if(efd < 0) {
            LOG_FATAL << "create eventfd fail!";
            abort();
        }
        return efd;
    }
    void ReadEventfd() {
        uint64_t res = 0;
        int ret = read(_eventfd, &res, sizeof(res));
        if(ret < 0) {
            if(errno == EINTR || errno == EAGAIN) { 
                return; 
            }
            LOG_FATAL << "read eventfd fail!";
            abort();
        }
        return;
    }
    void WakeUpEventFd() {
        uint64_t val = 1;
        int ret = write(_eventfd, &val, sizeof(val));
        if(ret < 0) {
            if(errno == EINTR) {
                return;
            }
            LOG_FATAL << "write eventfd fail!";
            abort();
        }
        return;
    }
private:
    std::thread::id _thread_id;     // brief: 线程id
    int _eventfd;                   // brief: _eventfd 唤醒IO事件监控有可能导致的阻塞
    std::unique_ptr<Channel> _event_channel;
    Poller _poller;                 // brief: 执行所有 channel 的事件监控
    TimeWheel _time_wheel;

    
    std::vector<Functor> _tasks;    // brief: 任务池
    std::mutex _mutex;
};