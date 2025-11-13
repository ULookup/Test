#include "EventLoop.hpp"

TimerTask::TimerTask(uint64_t id, uint32_t delay, const TaskFunc &cb)
        : _id(id), _timeout(delay), _task_cb(cb), _is_cancel(0)
        {}

    void TimerTask::SetRelease(const ReleaseFunc &cb) { _release_cb = cb; }

    void TimerTask::Cancel() { _is_cancel = true; }
    uint32_t TimerTask::DelayTime() { return _timeout; }

    TimerTask::~TimerTask(){
        if(!_is_cancel){
            _task_cb();
        }
        _release_cb();
    }

    void TimeWheel::Remove(uint64_t id){
        auto it = _timers.find(id);
        if(it != _timers.end()){
            _timers.erase(id);
        }
    }

    TimeWheel::TimeWheel(EventLoop *loop, int capacity):
        _capacity(capacity),
        _tick(0), 
        _timewheel(_capacity), 
        _loop(loop),
        _timerfd(CreateTimerFd()),
        _timer_channel(std::make_unique<Channel>(loop, _timerfd)) {
            _timer_channel->SetReadCallback(std::bind(&TimeWheel::OnTime, this));
            _timer_channel->EnableRead();
        }
    void TimeWheel::AddTimer(uint64_t id, uint32_t delay, const TaskFunc &cb) {
        _loop->RunInLoop(std::bind(&TimeWheel::AddTimerInLoop, this, id, delay, cb));
    }
    void TimeWheel::RefreshTimer(uint64_t id){
        _loop->RunInLoop(std::bind(&TimeWheel::RefreshTimerInLoop, this, id));
    }
    void TimeWheel::CancelTimer(uint64_t id){
       _loop->RunInLoop(std::bind(&TimeWheel::CancelTimerInLoop, this, id));
    }
    /* waring: 这个接口不能被外界使用者调用，只能在模块内，对应EventLoop线程内执行 */
    bool TimeWheel::HasTimer(uint64_t id) {
        auto it = _timers.find(id);
        if(it == _timers.end()) {
            return false;
        }
        return true;
    }

    int TimeWheel::CreateTimerFd() {
        int timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
        if(timerfd < 0) {
            return -1;
        }
        struct itimerspec itime;
        itime.it_value.tv_sec = 1;
        itime.it_value.tv_nsec = 0; // 第一次超时在1s后
        itime.it_interval.tv_sec = 1;
        itime.it_interval.tv_nsec = 0; // 第一次超时后，每次超时间隔时间
        timerfd_settime(timerfd, 0, &itime, NULL);
        return timerfd;
    }
    void TimeWheel::ReadTimeFd() {
        uint64_t times;
        int ret = read(_timerfd, &times, 8);
        if(ret < 0) {
            abort();
        }
        return;
    }
    void TimeWheel::RunTimerTask(){
        _tick = (_tick + 1) % _capacity;
        _timewheel[_tick].clear();
    }
    void TimeWheel::OnTime() {
        ReadTimeFd();
        RunTimerTask();
    }
    void TimeWheel::AddTimerInLoop(uint64_t id, uint32_t delay, const TaskFunc &cb){
        //SharedPtr ptr(new TimerTask(id, delay, cb));
        SharedPtr ptr = std::make_shared<TimerTask>(id, delay, cb);
        ptr->SetRelease(std::bind(&TimeWheel::Remove, this, id));
        int pos = (_tick + delay) % _capacity;
        _timewheel[pos].push_back(ptr);
        _timers[id] = WeakPtr(ptr);
    }
    void TimeWheel::RefreshTimerInLoop(uint64_t id){
        auto it = _timers.find(id);
        if(it == _timers.end()){
            return;
        }
        SharedPtr ptr = it->second.lock();
        int delay =  ptr->DelayTime();
        int pos = (_tick + delay) % _capacity;
        _timewheel[pos].push_back(ptr);
    }
    void TimeWheel::CancelTimerInLoop(uint64_t id){
        auto pos = _timers.find(id);
        if(pos == _timers.end()){
            return;
        }
        SharedPtr ptr = pos->second.lock();
        if(ptr){
            ptr->Cancel();
        }
    }