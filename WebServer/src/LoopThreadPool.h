#pragma once

#include "LoopThread.h"
#include <vector>

namespace webserver::src
{

class EventLoop;

class LoopThreadPool
{
public:
    LoopThreadPool(EventLoop *baseloop) : _thread_count(0), _next_loop_idx(0), _baseloop(baseloop) {}
    /* brief: 暴露给上层来设置线程数量 */
    void SetThreadCount(int count) { _thread_count = count; }
    /* brief: 创建线程池 */
    void Create() {
        if(_thread_count > 0) {
            _threads.resize(_thread_count);
            _loops.resize(_thread_count);
            for(int i = 0; i < _thread_count; ++i) {
                _threads[i] = new LoopThread(); // 此次可以用内存池优化
                _loops[i] = _threads[i]->GetLoop();
            }
        }
    }
    /* brief: 获取下一个EventLoop */
    EventLoop *NextLoop() {
        if(_thread_count == 0) return _baseloop;
        _next_loop_idx = (_next_loop_idx + 1) % _thread_count;
        return _loops[_next_loop_idx];
    }
private:
    int _thread_count; // 从属线程数
    int _next_loop_idx;
    EventLoop *_baseloop; // 主reactor，运行在主线程，如果从属线程数为0，则所有操作都在baseloop进行
    std::vector<LoopThread*> _threads; // 保存所有的LoopThread对象
    std::vector<EventLoop*> _loops; // 从属线程大于0，则从_loops种进行线程EventLoop分配
};

}