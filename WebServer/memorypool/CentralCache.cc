#include "CentralCache.h"
#include "PageCache.h"

CentralCache CentralCache::_sInst;

Span *CentralCache::GetOneSpan(SpanList& list, size_t size) {
    // step1. 查看当前spanlist里是否还有未分配对象的span
    Span *it = list.Begin();
    while(it != list.End()) {
        if(it->_freelist != nullptr) return it;
        it = it->_next;
    }
    // step2. 走到这里说明没有未分配的 span 了，找 pagecache 要
    // tips: 先把central cache的桶锁解掉，这样如果其它线程释放内存对象回来，不会阻塞住
    list._mtx.unlock();


    PageCache::GetInstance()->_pagemtx.lock();
    Span *span = PageCache::GetInstance()->NewSpan(SizeClass::NumMovePage(size));
    span->_isUse = true;
    span->_obj_size = size;
    PageCache::GetInstance()->_pagemtx.unlock();
    // 对获取的span进行切分，不需要加锁，因为其它线程访问不到这个span
    // 计算span的大块内存的起始地址和大块内存的大小（字节数）
    char *start = (char*)(span->_pageId << PAGE_SHIFT);
    size_t bytes = span->_page_num << PAGE_SHIFT;
    char *end = start + bytes;
    // 把大块内存切成自由链表
    //先切一块做头，方便尾插
    span->_freelist = start;
    start += size;
    void* tail = span->_freelist;

    while(start < end) {
        NextObj(tail) = start;
        tail = NextObj(tail); // tail = start
        start += size;
    }

    NextObj(tail) = nullptr;
    //切好span以后，需要挂到桶里面，要加锁
    list._mtx.lock();
    list.PushFront(span);

    return span;
}

size_t CentralCache::FetchRangeObj(void* &start, void* &end, size_t batchNum, size_t byteSize) {
    size_t index = SizeClass::Index(byteSize);
    _spanlists[index]._mtx.lock();

    Span* span = GetOneSpan(_spanlists[index], byteSize);
    assert(span);
    assert(span->_freelist);

    // 在 span 的 freelist 取需要的 batchNum 个内存块
    start = span->_freelist;
    end = start;
    size_t i = 0;
    size_t actualNum = 1;
    while( i < batchNum - 1 && NextObj(end) != nullptr) {
        end = NextObj(end);
        ++i;
        ++actualNum;
    }
    span->_freelist = NextObj(end);
    NextObj(end) = nullptr;
    span->_use_count += actualNum;

    _spanlists[index]._mtx.unlock();

    return actualNum;
}

void CentralCache::ReleaseListToSpans(void *start, size_t size) {
    size_t index = SizeClass::Index(size);
    _spanlists[index]._mtx.lock();

    while(start) {
        void *next = NextObj(start);

        Span *span = PageCache::GetInstance()->MapObjToSpan(start);
        NextObj(start) = span->_freelist;
        span->_freelist = start;
        span->_use_count--;
        if(span->_use_count == 0) {
            //说明span切分出去的所有小块内存都回来了，可以回收给pagecache，pagecache可以尝试前后页合并
            _spanlists[index].Erase(span);
            span->_freelist = nullptr;
            span->_next = nullptr;
            span->_prev = nullptr;
            //释放span给pagecache时，使用pagecache的锁
            //这时解掉桶锁
            _spanlists[index]._mtx.unlock();

            PageCache::GetInstance()->_pagemtx.lock();
            PageCache::GetInstance()->ReleaseSpanToPageCache(span);
            PageCache::GetInstance()->_pagemtx.unlock();

            _spanlists[index]._mtx.lock();
        }

        start = next;
    }
    
    _spanlists[index]._mtx.unlock();
}