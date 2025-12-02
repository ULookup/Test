#pragma once

#include <mutex>
#include <thread>
#include <condition_variable>

namespace webserver::src
{

class EventLoop;

class LoopThread
{
public:
    /* brief: 创建 EventLoop 线程，设置好线程执行的入口函数，但先不绑定对应的EventLoop */
    LoopThread() :_loop(nullptr), _thread(std::thread(&LoopThread::ThreadEntry, this)) {}
    /* brief: 返回当前线程绑定的EventLoop */
    EventLoop *GetLoop() {
        EventLoop *loop = nullptr;
        {
            std::unique_lock<std::mutex> lock(_mutex); //加锁
            _cond.wait(lock, [&](){ return &_loop != nullptr; }); // 这里的lamda表达式是“谓词”，用来防止虚假唤醒，只要返回true时才真正结束等待
            loop = _loop;
        }
        return loop;
    }
private:
    /* brief: 线程执行的入口 routine 函数*/
    void ThreadEntry() {
        EventLoop loop; // 这里用到了RAII思想，该线程绑定的EventLoop的生命周期与线程绑定，线程销毁，它的EventLoop随之销毁
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _loop = &loop;

            _cond.notify_all(); //唤醒阻塞的线程
        }
        _loop.Start();
    }
private:
    /* 锁用于实现 _loop 获取的同步关系，避免线程创建了，但是 _loop 还没有实例化之前去获取 _loop */
    std::mutex _mutex;  // 互斥锁
    std::condition_variable _cond;  //条件变量
    EventLoop *_loop; // 该线程绑定的EventLoop
    std::thread _thread; // 线程
};

}