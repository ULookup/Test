#include "TimeWheel.h"

namespace webserver::src
{
/* brief: 计时任务构造函数 */
TimerTask::TimerTask(uint64_t id, uint32_t delay, const TaskFunc &cb)
    : _id(id), _timeout(delay), _task_cb(cb)
    {}
/* brief: 计时任务析构函数，析构时如果任务没取消，则执行定时任务。之后执行计时任务释放回调函数 */
TimerTask::~TimerTask() {
    if(!_is_cancel) {
        _task_cb();
    }
    _release_cb();
}
// ========== TimeWheel ==========
TimeWheel::TimeWheel(EventLoop *loop, int capacity = 60) 
    : _capacity(60), _tick(0), _timewheel(_capacity), _loop(loop),
    _timerfd(CreateTimerFd()), _timer_channel(std::make_unique<Channel>(loop, _timerfd))
    {
        _timer_channel->SetReadCallback(std::bind(&TimeWheel::OnTime, this));
        _timer_channel->EnableRead();
    }

void TimeWheel::AddTimer(uint64_t id, uint32_t delay, const TaskFunc &cb) {
    _loop->RunInLoop(std::bind(&TimeWheel::AddTimerInLoop, this, id, delay, cb));
}

void TimeWheel::RefreshTimer(uint64_t id) {
    _loop->RunInLoop(std::bind(&TimeWheel::RefreshTimerInLoop, this, id));
}

void TimeWheel::CancelTimer(uint64_t id) {
    _loop->RunInLoop(std::bind(&TimeWheel::CancelTimerInLoop, this, id));
}

bool TimeWheel::HasTimer(uint64_t id) {
    auto it = _timers.find(id);
    if(it == _timers.end()) {
        return false;
    }
    return true;
}

void TimeWheel::Remove(uint64_t id) {
    auto it = _timers.find(id);
    if(it != _timers.end()) {
        _timers.erase(id);
    }
}

int TimeWheel::CreateTimerFd() {
    int timerfd = timerfd_create(CLOCK_MONOTONIC, 0); // 调用系统调用创建一个定时器描述符
    if(timerfd < 0) {
        // create timerfd failed
        abort();
    }
    struct itimerspec itime;
    // 第一次超时在1s后
    itime.it_value.tv_sec = 1;
    itime.it_value.tv_nsec = 0; 
    // 第一次超时后，每次超时间隔时间
    itime.it_interval.tv_sec = 1;
    itime.it_interval.tv_nsec = 0;
    // 设置计时时间
    timerfd_settime(timerfd, 0, &itime, NULL);

    return timerfd;
}
/*  brief: 读取计时器事件描述符 */
int TimeWheel::ReadTimeFd() {
    uint64_t times;
    // 有可能因为其它描述符的事件处理较长，然后在处理定时器描述符事件的时候，有的任务超时了很多次
    // read读取的times就是从上一次read之后超时的次数
    int ret = read(_timerfd, &times, 8);
    if(ret < 0) {
        abort;
    }
    return times;
}
/* brief: 执行计时任务的函数 */
void TimeWheel::RunTimerTask() {
    _tick = (_tick + 1) % _capacity;
    _timewheel[_tick].clear(); // 将这个时间格里的vector清空，对应的计时任务被析构，从而执行定时任务

}
/* brief: 同步时间 */
void TimeWheel::OnTime() {
    int times = ReadTimeFd();
    for(int i = 0; i < times; ++i) {
        RunTimerTask(); //同步时间，然后将超时了的任务全部执行
    }
}
/* brief: 添加定时任务的实际执行 */
void TimeWheel::AddTimerInLoop(uint64_t id, uint32_t delay, const TaskFunc &cb) {
    SharedPtr ptr = std::make_shared<TimerTask>(id, delay, cb); // 创建一个定时任务的共享指针，这样只要计数不为0，就不会被析构 
    ptr->SetRelease(std::bind(&TimeWheel::Remove, this, id));   // 设置定时任务释放函数，在定时任务析构时，把它从哈希表内移除
    int pos = (_tick + delay) % _capacity;                      // 定时任务插入位置就是秒针当前指向的位置 + 要定的时间
    _timewheel[pos].push_back(ptr);                             // 将定时任务插入对应时间的任务桶中
    _timers[id] = WeakPtr(ptr);                                 // 在定时任务表中添加这一定时任务（注 weakptr 不会暂用引用计数
}
/* brief: 刷新定时任务的实际执行 */
void TimeWheel::RefreshTimerInLoop(uint64_t id) {
    auto it = _timers.find(id);
    if(it == _timers.end()) {
        // 不存在这个定时任务
        return;
    }
    SharedPtr ptr = it->second.lock(); // 从 WeakPtr 中创建一个 SharedPtr
    int delay = ptr->DelayTime();
    int pos = (_tick + delay) % _capacity;
    _timewheel[pos].push_back(ptr);
}
/* brief: 关闭定时任务的实际执行 */
void TimeWheel::CancelTimer(uint64_t id) {
    auto pos = _timers.find(id);
    if(pos == _timers.end()) {
        // 不存在这个定时任务
        return;
    }
    SharedPtr ptr = pos->second.lock();
    if(ptr) {
        ptr->Cancel();
    }
}

}