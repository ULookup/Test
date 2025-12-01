#pragma once

#include "Common.h"

// 单例模式
class CentralCache
{
public:
    static CentralCache *GetInstance() { return &_sInst; }

    Span* GetOneSpan(SpanList& list, size_t size);
    /* 从中心缓存拿一定量对象 */
    size_t FetchRangeObj(void* &start, void* &end, size_t batchNum, size_t byteSize);
    /* brief：将一定数量的对象释放到 span 里 */
    void ReleaseListToSpans(void *start, size_t size);
private:
    CentralCache() = default;

    CentralCache(const CentralCache&) = delete;
    CentralCache &operator=(const CentralCache&) = delete;

    static CentralCache _sInst;
private:
    SpanList _spanlists[NFREE_LISTS];
};