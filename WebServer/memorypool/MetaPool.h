#pragma once

#include "Common.h"

template<class T>
class MetaPool
{
public:
    T *New() {
        T *obj = nullptr;

        //优先把还回来的内存块对象，重复使用
        if(_freeLists) {
            void *next = NextObj(_freeLists);
            obj = (T*)_freeLists;
            _freeLists = next;
        } else {
            // 剩余内存不够一个对象大小时，则重开大块空间
            if(_remainBytes < sizeof(T)) {
                _remainBytes = 128 * 1024;
                _memory = (char*)SystemAlloc(_remainBytes >> PAGE_SHIFT);
                if(_memory == nullptr) {
                    throw std::bad_alloc();
                }
            }
            obj = (T*)_memory;
            size_t objsize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
            _memory += objsize;
            _remainBytes -= objsize;
        }
        // 定位new，显示调用T的构造函数初始化
        new(obj)T;

        return obj;
    }

    void Delete(T* obj) {
        //显示调用析构函数清理对象
        obj->~T();

        // 头插
        NextObj(obj) = _freeLists;
        _freeLists = obj;
    }
private:
    char *_memory = nullptr; // 指向大块内存的指针
    size_t _remainBytes = 0; // 大块内存在切分过程中剩余字节数

    void *_freeLists = nullptr;
};