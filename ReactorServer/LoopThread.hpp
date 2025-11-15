#pragma once

#include "Acceptor.hpp"
#include <thread>
#include <condition_variable>

class LoopThread
{
public:
    /* brief: 创建线程，设定线程入口函数 */
    LoopThread() : _loop(nullptr), _thread(std::thread(&LoopThread::ThreadEntry, this)) {}
    /* brief: 返回当前线程关联的 EventLoop 对象指针 */
    EventLoop *GetLoop() {
        EventLoop *loop = nullptr;
        {
            std::unique_lock<std::mutex> lock(_mutex);          //加锁
            _cond.wait(lock, [&](){return _loop != nullptr; }); //loop为nullptr就一直阻塞，等待被唤醒
            loop = _loop;
        }
        return loop;
    }
private:
    /* brief: 实例化 EventLoop 对象, 并且开始运行 EventLoop 模块的功能，唤醒 _cond 上可能阻塞的线程 */
    void ThreadEntry() {
        EventLoop loop; // RAII思想: 由 LoopThread 来管理 EventLoop 的生命周期
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _loop = &loop;

            _cond.notify_all(); //唤醒阻塞的线程
        }
        loop.Start();
    }
private:
    /* 用于实现 _loop 获取的同步关系，避免线程创建了，但是 _loop 还没有实例化之前去获取 _loop */
    std::mutex _mutex;              //互斥锁
    std::condition_variable _cond;  //条件变量
    EventLoop *_loop;
    std::thread _thread;
};