#pragma once

#include <cstdint>
#include <sys/timerfd.h>
#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>

using TaskFunc = std::function<void()>;
using ReleaseFunc = std::function<void()>;

class TimerTask
{
public:
    TimerTask(uint64_t id, uint32_t delay, const TaskFunc &cb)
        : _id(id), _timeout(delay), _task_cb(cb), _is_cancel(0)
        {}

    void SetRelease(const ReleaseFunc &cb) { _release_cb = cb; }

    void Cancel() { _is_cancel = true; }
    uint32_t DelayTime() { return _timeout; }

    ~TimerTask(){
        if(!_is_cancel){
            _task_cb();
        }
        _release_cb();
    }
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
private:
    void Remove(uint64_t id){
        auto it = _timers.find(id);
        if(it != _timers.end()){
            _timers.erase(id);
        }
    }
public:
    TimeWheel(int capacity = 60)
        : _capacity(capacity), _tick(0), _timewheel(_capacity)
        {}
    void AddTimer(uint64_t id, uint32_t delay, const TaskFunc &cb){
        //SharedPtr ptr(new TimerTask(id, delay, cb));
        SharedPtr ptr = std::make_shared<TimerTask>(id, delay, cb);
        ptr->SetRelease(std::bind(&TimeWheel::Remove, this, id));
        int pos = (_tick + delay) % _capacity;
        _timewheel[pos].push_back(ptr);
        _timers[id] = WeakPtr(ptr);
    }
    void RefreshTimer(uint64_t id){
        auto it = _timers.find(id);
        if(it == _timers.end()){
            return;
        }
        SharedPtr ptr = it->second.lock();
        int delay =  ptr->DelayTime();
        int pos = (_tick + delay) % _capacity;
        _timewheel[pos].push_back(ptr);
    }
    void RunTimerTask(){
        _tick = (_tick + 1) % _capacity;
        _timewheel[_tick].clear();
    }
    void CancelTimer(uint64_t id){
        auto pos = _timers.find(id);
        if(pos == _timers.end()){
            return;
        }
        SharedPtr ptr = pos->second.lock();
        if(ptr){
            ptr->Cancel();
        }
    }
private:
    int _capacity;
    int _tick;
    std::vector<std::vector<SharedPtr>> _timewheel;
    std::unordered_map<uint64_t, WeakPtr> _timers;
};