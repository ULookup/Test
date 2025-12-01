#include "ThreadCache.h"
#include "CentralCache.h"

void *ThreadCache::Allocate(size_t size) {
    assert(size <= MAX_BYTES);
    //
    size_t alignsize = SizeClass::RoundUp(size);
    size_t index = SizeClass::Index(size);
    
    if(!_freeLists[index].Empty()) {
        return _freeLists[index].Pop();
    } else {
        return FetchFromCentralCache(index, alignsize);
    }
}

void ThreadCache::Deallocate(void *ptr, size_t size) {
    assert(ptr);
    assert(size <= MAX_BYTES);

    //找到映射的桶，插进去
    size_t index = SizeClass::Index(size);
    _freeLists[index].Push(ptr);

    //当链表长度大于一次批量申请的内存时，就开始还一段给 centralCache
    if(_freeLists[index].Size() >= _freeLists[index].MaxSize()) {
        ListTooLong(_freeLists[index], size);
    }

    //std::cout << "Dealloc size=" << size 
    //      << " index=" << index 
    //      << " freelist size=" << _freeLists[index].Size() 
    //      << std::endl;

}

void *ThreadCache::FetchFromCentralCache(size_t index, size_t size) {
    // 慢开始反馈调节算法——有点像TCP的滑动窗口/拥塞控制
    // 1.最开始不会一次想centralcache批量要太多，太多了用不完
    // 2.如果不要这个size大小内存需求，batchNum就会不断增长，直到上线
    // 3.size越大，一次想centralcache要的batchNum就越小
    // 4.如果size越小，一次向centralcache要的batchNum就越大（慢增长）
    size_t batchNum = std::min(_freeLists[index].MaxSize(), SizeClass::NumMoveSize(size));

    if(_freeLists[index].MaxSize() == batchNum) _freeLists[index].MaxSize() += 1;

    void *start = nullptr;
    void *end = nullptr;
    size_t actualNum =  CentralCache::GetInstance()->FetchRangeObj(start, end, batchNum, size);
    assert(actualNum > 0);

    if(actualNum == 1) {
        assert(start == end);
        return start;
    } else {
        _freeLists[index].PushRange(NextObj(start), end, actualNum - 1);
        return start;
    }

    return nullptr;
}

void ThreadCache::ListTooLong(FreeList &list, size_t size) {
    void *start = nullptr;
    void *end = nullptr;
    list.PopRange(start, end, list.MaxSize());

    CentralCache::GetInstance()->ReleaseListToSpans(start, size);
}
