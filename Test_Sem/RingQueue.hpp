#ifndef __RINGQUEUE_H__
#define __RINGQUEUE_H__

#include <vector>
#include "Sem.hpp"
#include "mutex.h"

namespace icepop
{
    template <class T>
    class ringQueue
    {
    public:
        ringQueue(int cap) :
            _cap(cap), _ring_queue(_cap), 
            _spaceSem(cap), _dataSem(0),
            _p_index(0), _c_index(0),
            _p_lock(), _c_lock() 
        {}
        ~ringQueue() {}

        void Pop(T* out)
        {
            {
                LockGuard lockguard(_c_lock);
                _dataSem.P();      //申请数据信号量
#ifdef DEBUG
                std::cout << "[DEBUG]consumer P() success!" << std::endl;
                std::cout << "[DEBUG]consumer index: " << _c_index << std::endl;
#endif
                *out = _ring_queue[_c_index++];
#ifdef DEBUG
                std::cout << "[DEBUG]consumer getdata success!" << std::endl;
#endif
                _c_index %= _cap;
            }
#ifdef DEBUG
                std::cout << "[DEBUG]consumer V() success!" << std::endl;
#endif
            _spaceSem.V();      //让空间信号量增加，表明可以写入
        }
        void Enqueue(const T& in)
        {
            {
                LockGuard lockguard(_p_lock);
                _spaceSem.P();      //申请空间信号量
#ifdef DEBUG
                std::cout << "[DEBUG]product P() success!" << std::endl;
                std::cout << "[DEBUG]product index: " << _p_index << std::endl;
#endif
                _ring_queue[_p_index++] = in;
#ifdef DEBUG
                std::cout << "[DEBUG]product enqueue success!" << std::endl;
#endif
                _p_index %= _cap;
            }
            _dataSem.V();      //让数据信号量增加，表明可以读取
#ifdef DEBUG
                std::cout << "[DEBUG]product V() success!" << std::endl;
#endif
        }
#ifdef DEBUG
        void Print()
        {
            std::cout << _ring_queue.size() << std::endl;
            std::cout << _cap << std::endl;
            std::cout << _p_index << std::endl;
            std::cout << _c_index << std::endl;
        }
#endif
    private : 
        int _cap;                      //初始化列表的初始顺序是按照定义顺序来的！！！
        std::vector<T> _ring_queue;    //初始化列表的初始顺序是按照定义顺序来的！！！
                                       //初始化列表的初始顺序是按照定义顺序来的！！！
        int _p_index;
        int _c_index;

        Sem _spaceSem;
        Sem _dataSem;

        mutex _p_lock;
        mutex _c_lock;
    };
}

#endif