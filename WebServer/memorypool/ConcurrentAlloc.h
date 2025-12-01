#pragma once

#include "Common.h"
#include "ThreadCache.h"
#include "PageCache.h"

static MetaPool<ThreadCache> tcPool;

static std::mutex tcPoolMtx;

static void * ConcurrentAlloc(size_t size) {

    if(size > MAX_BYTES) {
        size_t alignSize = SizeClass::RoundUp(size);
        size_t kpage = alignSize >> PAGE_SHIFT;
        
        PageCache::GetInstance()->_pagemtx.lock();
        Span *span = PageCache::GetInstance()->NewSpan(kpage);
        span->_obj_size = size;
        PageCache::GetInstance()->_pagemtx.unlock();

        void *ptr = (void*)(span->_pageId << PAGE_SHIFT);
        return ptr;
    } else {
        if(pTLSThreadCache == nullptr) {
            std::lock_guard<std::mutex> lk(tcPoolMtx);
            //pTLSThreadCache = new ThreadCache;
            pTLSThreadCache = tcPool.New();
        }

        return pTLSThreadCache->Allocate(size);
    }
}

static void ConcurrentFree(void *ptr) {
    Span *span = PageCache::GetInstance()->MapObjToSpan(ptr);
    size_t size = span->_obj_size;
    //std::cout << "span->_obj_size = " << span->_obj_size << std::endl;

    if(size > MAX_BYTES) {
        PageCache::GetInstance()->_pagemtx.lock();
        PageCache::GetInstance()->ReleaseSpanToPageCache(span);
        PageCache::GetInstance()->_pagemtx.unlock();
    } else {
        assert(pTLSThreadCache);

        pTLSThreadCache->Deallocate(ptr, size);
    }

}