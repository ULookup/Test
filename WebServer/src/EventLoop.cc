#include "EventLoop.h"

namespace webserver::src

{

EventLoop::EventLoop(): _thread_id(std::this_thread::get_id()),
                        _eventfd(CreateEventFd()),
                        _event_channel(std::make_unique<Channel>(this, _eventfd)),
                        _time_wheel(this)
{
    /* notes: 给 _eventfd 添加可读事件回调函数，读取 _eventfd 事件通知次数 */
    _event_channel->SetReadCallback(std::bind(&EventLoop::ReadEventfd, this));
    /* notes: 启动 _eventfd 读事件监控 */
    _event_channel->EnableRead();
}
/* brief: 判断将要执行的任务是否属于该EventLoop对应的线程，如果是就直接执行，如果不是就压入该EventLoop队列 */
void EventLoop::RunInLoop(const Functor &cb) {
    if(IsInLoop()) {
        return cb();
    }
    return PushInLoop(cb);
}
/* brief: 将需要该EventLoop执行的任务压入任务池 */
void EventLoop::PushInLoop(const Functor &cb) {
    {
        std::unique_lock<std::mutex> _lock(_mutex);
        _tasks.push_back(cb);
    }
    //唤醒可能因为没有事件就绪，而在epoll_wait阻塞的该eventloop对应的线程（给eventfd写一个数据，触发可读事件）
    WakeUpEventFd();
}

// ===================== EventLoop 的 Loop 循环 ======================

/* brief: EventLoop 的 Loop 循环所在 */
void EventLoop::Start() {
    while(true) {
        // step1: 事件监控
        std::vector<Channel*> actives;
        _poller.Poll(actives); // 输出型参数，_poller返回活跃的Channel，channel保存了revents
        // step2: 就绪事件处理
        for(auto &channel : actives) {
            channel->HandlerEvent(); // channel根据revent里的就绪事件，执行相应的回调函数
        }
        // step3: 执行任务
        RunAllTask();
    }
}

// ======================= PRIVATE =============================

/* brief: 执行该EventLoop任务池的所有任务 */
void EventLoop::RunAllTask() {
    std::vector<Functor> functor;
    {
        std::unique_lock<std::mutex> _lock(_mutex);
        _tasks.swap(functor);
    }
    for(auto &f : functor) f();

    return;
}
/* brief: 创建eventfd */
int EventLoop::CreateEventFd() {
    int efd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if(efd < 0) {
        // create eventfd fail!
        abort();
    }
    return efd;
}
/* brief: 读取EventFd */
void EventLoop::ReadEventfd() {
    uint64_t res = 0;
    int ret = read(_eventfd, &res, sizeof(res));
    if(ret < 0) {
        if(errno == EINTR || errno == EAGAIN) {
            return;
        }
        // read eventfd fail!
        abort();
    }
    return;
}
/* brief: 通过EventFd唤醒 */
void EventLoop::WakeUpEventFd() {
    uint64_t val = 1;
    int ret = write(_eventfd, &val, sizeof(val));
    if(ret < 0) {
        if(errno == EINTR) {
            return;
        }
        // write eventfd fail!
        abort();
    }
    return;
}

}