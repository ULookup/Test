#pragma once

#include "Common.h"
#include "MetaPool.h"
#include "PageMap.h"

class PageCache
{
public:
    static PageCache *GetInstance() { return &_sInst; }

    /* brief: 获取 k 页 span */
    Span *NewSpan(size_t k);

    Span* MapObjToSpan(void *obj);

    void ReleaseSpanToPageCache(Span *span);

    std::mutex _pagemtx;
private:
    //PageCache() : _idSpanMap(SystemAllocWrapper) {}
    PageCache() = default;
    PageCache(const PageCache&) = delete;
    PageCache &operator=(const PageCache&) = delete;
private:
    SpanList _spanlists[NPAGES];
    MetaPool<Span> _spanPool;
    //std::unordered_map<PAGE_ID, Span*> _idSpanMap; 
    PageMap _idSpanMap;

    static PageCache _sInst;
};