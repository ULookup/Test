#pragma once

#include "Channel.hpp"
#include <cstdint>
#include <sys/timerfd.h>
#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>
#include <unistd.h>

class EventLoop;

using TaskFunc = std::function<void()>;
using ReleaseFunc = std::function<void()>;

class TimerTask
{
public:
    TimerTask(uint64_t id, uint32_t delay, const TaskFunc &cb);

    void SetRelease(const ReleaseFunc &cb);

    void Cancel();
    uint32_t DelayTime();

    ~TimerTask();
private:
    bool _is_cancel;
    uint64_t _id;   //定时器任务id
    uint32_t _timeout; //定时器超时时间
    TaskFunc _task_cb;  //定时器对象要执行的定时任务
    ReleaseFunc _release_cb;
};

using WeakPtr = std::weak_ptr<TimerTask>;
using SharedPtr = std::shared_ptr<TimerTask>;

class TimeWheel
{
public:
    TimeWheel(EventLoop *loop, int capacity = 60);
    void AddTimer(uint64_t id, uint32_t delay, const TaskFunc &cb);
    void RefreshTimer(uint64_t id);
    void CancelTimer(uint64_t id);
    /* waring: 这个接口不能被外界使用者调用，只能在模块内，对应EventLoop线程内执行 */
    bool HasTimer(uint64_t id);
private:
    void Remove(uint64_t id);

    static int CreateTimerFd();
    void ReadTimeFd();
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

    EventLoop* _loop;
    int _timerfd; //定时器描述符
    std::unique_ptr<Channel> _timer_channel;
};