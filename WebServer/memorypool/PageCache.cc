#include "PageCache.h"

PageCache PageCache::_sInst;


Span *PageCache::NewSpan(size_t k) {
    assert(k > 0);

    if(k > NPAGES - 1) {
        // 大于 128 页，直接向堆申请
        void *ptr = SystemAlloc(k);
        Span *span = _spanPool.New();
        span->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;
        span->_page_num = k;

        //_idSpanMap[span->_pageId] = span;
        _idSpanMap.set(span->_pageId, span);
        // 映射 new span（只写 begin/end）

        return span;
    }
    // 先检查一下第 k 个桶里面有没有span
    if(!_spanlists[k].Empty()) {
        Span *kSpan = _spanlists[k].PopFront();

        for(PAGE_ID i = 0; i < kSpan->_page_num; ++i) {
            //_idSpanMap[kSpan->_pageId + i] = kSpan;
            _idSpanMap.set(kSpan->_pageId + i, kSpan);
        }
        return kSpan;
    }

    // 检查一下后面的桶里面有没有span，如果有，可以把它进行切分
    for(size_t i = k + 1; i < NPAGES; ++i) {
        if(!_spanlists[i].Empty()) {
            Span *nSpan = _spanlists[i].PopFront();
            Span *kSpan = _spanPool.New();
            // 在nSpan的头部切一个k页下来
            // k页span返回
            // nspan再挂到映射的位置 
            kSpan->_pageId = nSpan->_pageId;
            kSpan->_page_num = k;

            nSpan->_pageId += k;
            nSpan->_page_num -= k;

            _spanlists[nSpan->_page_num].PushFront(nSpan);
            //存储 nSpan 的首尾页号跟nSpan映射，方便pagecache回收内存时进行的合并查找
            //_idSpanMap[nSpan->_pageId] = nSpan; 
            //_idSpanMap[nSpan->_pageId + nSpan->_page_num - 1] = nSpan;
            _idSpanMap.set(nSpan->_pageId, nSpan);
            _idSpanMap.set(nSpan->_pageId + nSpan->_page_num - 1, nSpan);

            //建立 id 和 span的映射，方便centralcache回收小块内存时，查找对应span
            for(PAGE_ID i = 0; i < kSpan->_page_num; ++i) {
                //_idSpanMap[kSpan->_pageId + i] = kSpan;
                _idSpanMap.set(kSpan->_pageId + i, kSpan);
            }
            
            return kSpan;
        }
    }
    // 走到这里说明没有大页span
    // 这个时候找堆要一个128页的span
    Span *bigSpan = _spanPool.New();
    void *ptr = SystemAlloc(NPAGES - 1);
    bigSpan->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;
    bigSpan->_page_num = NPAGES - 1;

    _spanlists[bigSpan->_page_num].PushFront(bigSpan);

    return NewSpan(k);
} 

Span* PageCache::MapObjToSpan(void *obj) {
    PAGE_ID id = ((PAGE_ID)obj >> PAGE_SHIFT);

    /* std::unique_lock<std::mutex> lock(_pagemtx);
    auto ret = _idSpanMap.find(id);
    if(ret != _idSpanMap.end()) {
        return ret->second;
    } else {
        assert(false);
        return nullptr;
    } */

    auto ret = (Span*)_idSpanMap.get(id);
    assert(ret != nullptr);
    return ret;
}

void PageCache::ReleaseSpanToPageCache(Span *span) {
    // 大于 128 page，直接还给堆
    if(span->_page_num > NPAGES - 1) {
        void *ptr = (void*)(span->_pageId << PAGE_SHIFT);
        SystemFree(ptr, span->_page_num << PAGE_SHIFT);
        //delete span;
        _spanPool.Delete(span);

        return;
    }
    // 对 span 向前向后合并
    while(true) {
        //向前合并
        PAGE_ID prevId = span->_pageId - 1;
        /*auto ret = _idSpanMap.find(prevId);
        if(ret == _idSpanMap.end()) {
            // 前面的页号没有，不合并了
            break;
        } */
       auto ret = (Span*)_idSpanMap.get(prevId);
       if(ret == nullptr) {
            break;
       }
        Span *prevSpan = ret;
        if(prevSpan->_isUse == true) {
            // 前面相邻页的span在使用，不合并了
            break;
        }
        if(prevSpan->_page_num + span->_page_num > NPAGES - 1) {
            // 合并出超过 128 的span没办法管理，不合并
            break;
        }

        span->_pageId = prevSpan->_pageId;
        span->_page_num += prevSpan->_page_num;

        _spanlists[prevSpan->_page_num].Erase(prevSpan);
        //delete prevSpan;
        _spanPool.Delete(prevSpan);
    }
    // 向后合并
    while(true) {
        PAGE_ID nextId = span->_pageId + span->_page_num;
        /* auto ret = _idSpanMap.find(nextId);
        if(ret == _idSpanMap.end()) {
           break;
        } */
        auto ret = (Span*)_idSpanMap.get(nextId);
        if(ret == nullptr) {
            break;
        }
        Span *nextSpan = ret;
        if(nextSpan->_isUse == true) {
            break;

        }
        if(nextSpan->_page_num + span->_page_num > NPAGES - 1) {
            // 合并出超过 128 的span没办法管理，不合并
            break;
        }

        span->_page_num += nextSpan->_page_num;

        _spanlists[nextSpan->_page_num].Erase(nextSpan);
        //delete nextSpan;
        _spanPool.Delete(nextSpan);

    } 

    _spanlists[span->_page_num].PushFront(span);
    span->_isUse = false;
    //_idSpanMap[span->_pageId] = span;
    //_idSpanMap[span->_pageId + span->_page_num - 1] = span;
    _idSpanMap.set(span->_pageId, span);
    _idSpanMap.set(span->_pageId + span->_page_num - 1, span);
    
}