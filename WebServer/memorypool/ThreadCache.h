#pragma once

#include "Common.h"

class ThreadCache
{
public:
    // brief: 申请和释放对象
    void *Allocate(size_t size);
    void Deallocate(void *ptr, size_t size);

    // brief: 从中心缓存获取对象
    void *FetchFromCentralCache(size_t index, size_t size);

    // brief: 释放对象时，链表过长时，回收内存到中心缓存
    void ListTooLong(FreeList &list, size_t size);
private:
    FreeList _freeLists[NFREE_LISTS];
};

static __thread ThreadCache* pTLSThreadCache = nullptr;