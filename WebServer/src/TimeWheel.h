#pragma once

#include "EventLoop.h"
#include <memory>
#include <sys/timerfd.h>
#include <unistd.h>

namespace webserver::src
{

using TaskFunc = std::function<void()>;
using ReleaseFunc = std::function<void()>;

class TimerTask 
{
public:
    TimerTask(uint64_t id, uint32_t delay, const TaskFunc &cb);
    /* brief: 设置释放定时器回调 */
    void SetRelease(const ReleaseFunc &cb) { _release_cb = cb; }
    /* brief: 取消 */
    void Cancel() { _is_cancel = true; }
    /* brief: 超时时间 */
    uint32_t DelayTime() { return _timeout; }

    /* brief: 时间到了析构定时器任务，在析构函数种执行定时任务 */
    ~TimerTask();
private:
    bool _is_cancel;
    uint64_t _id;      //定时器任务id
    uint32_t _timeout; //定时器超时时间
    TaskFunc _task_cb; //定时器对象要执行的定时任务
    ReleaseFunc _release_cb;
};

using WeakPtr = std::weak_ptr<TimerTask>;
using SharedPtr = std::shared_ptr<TimerTask>;

class TimeWheel
{
public:
    TimeWheel(EventLoop *loop, int capacity = 60);
    /* brief: 添加定时任务 */
    void AddTimer(uint64_t id, uint32_t delay, const TaskFunc &cb);
    /* brief: 刷新定时器，体现为在时间轮新增一个定时任务 */
    void RefreshTimer(uint64_t id);
    /* brief: 取消定时器 */
    void CancelTimer(uint64_t id);
    /* waring: 这个接口不能被外界使用者调用，只能在模块内，对应EventLoop线程内执行 */
    /* brief: 检测有无定时任务 */
    bool HasTimer(uint64_t id);
private:
    /* brief: 移除定时任务 */
    void Remove(uint64_t id);
    /* brief: 创建TimerFd，操作系统底层会帮我们计时，时间到了会通过这个fd通知我们，和eventfd有异曲同工 */
    static int CreateTimerFd();
    int ReadTimeFd();
    /* brief: timerfd 被触发，进程就执行一次RunTimerTask，即让指针移动一次，执行一下所有过期定时任务 */
    void RunTimerTask();
    void OnTime();
    void AddTimerInLoop(uint64_t id, uint32_t delay, const TaskFunc &cb);
    void RefreshTimerInLoop(uint64_t id);
    void CancelTimerInLoop(uint64_t id);
private:
    int _capacity;
    int _tick;
    std::vector<std::vector<SharedPtr>> _timewheel;
    std::unordered_map<uint64_t, WeakPtr> _timers;

    EventLoop *_loop;
    int _timerfd; //定时器描述符号，实现内核每隔一段时间，给进程一次超时事件
    std::unique_ptr<Channel> _timer_channel;
};

}