#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <cassert>
#include <pthread.h>
#include <unistd.h>
#include <mutex>
#include <sys/mman.h>

static const size_t MAX_BYTES = 256*1024;
static const size_t NFREE_LISTS = 208;
static const size_t NPAGES = 129;
static const size_t PAGE_SHIFT = 12;

#ifdef __x86_64__
    typedef unsigned long long PAGE_ID;
#elif __i386__
    typedef size_t PAGE_ID;
#endif

inline static void *SystemAlloc(size_t kpage) {
#ifdef __WIN32
    void* ptr = VirtualAlloc(0, kpage << 13, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
    void *ptr = mmap(NULL, kpage << 13, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
    if(ptr == nullptr) throw std::bad_alloc();

    return ptr;
}

inline static void SystemFree(void *ptr, size_t length) {
#ifdef _WIN32
	VirtualFree(ptr, 0, MEM_RELEASE);
#else
	munmap(ptr, length);
#endif
}

static void* &NextObj(void *obj) {
    return *(void**)obj;
}

/* brief: 管理切分好的小对象的自由链表 */
class FreeList
{
public:
    void Push(void * obj) {
        //头插
        //*(void**)obj = _freelist; 
        NextObj(obj) = _freelist;
        _freelist = obj;
        _size++;
    }

    void PushRange(void *start, void *end, size_t n) {
        NextObj(end) = _freelist;
        _freelist = start;
        _size += n;
    }

    void *Pop() {
        assert(_freelist);
        //头删
        void *obj = _freelist;
        _freelist = NextObj(obj);
        _size--;

        return obj;
    }

    void PopRange(void *&start, void *&end, size_t n) {
        assert(n <= _size);
        start = _freelist;
        end = start;
        for(size_t i = 0; i < n - 1; ++i) {
            end = NextObj(end);
        }

        _freelist = NextObj(end);
        NextObj(end) = nullptr;
        _size -= n;
    }

    size_t Size() { return _size; }

    bool Empty() { return _freelist == nullptr; }



    size_t& MaxSize() { return _maxsize; }
private:
    void *_freelist = nullptr;
    size_t _maxsize = 1;
    size_t _size = 0;
};

// 计算对象大小的对齐映射规则
class SizeClass
{
public:
    // 整体控制在最大10%左右的内碎片浪费
    // [1, 128]                  8 byte对齐         freelist[0,16)   
    // [128+1, 1024]             16 byte对齐        freelist[16,72)
    // [1024+1, 8*1024]          128 byte对齐       freelist[72,128)
    // [8*1024+1,64*1024]        1024 byte对齐      freelist[128,184)
    // [64*1024+1,256*1024]      8*1024 byte对齐    freelist[184,208)

    //size_t _RoundUp(size_t size, size_t AlignNum) {
    //    size_t alignsize = 0;
    //    if(size % 8 != 0) {
    //        alignsize = (size / AlignNum + 1) * AlignNum;
    //    } else {
    //        alignsize = size;
    //    }
    //    return alignsize;
    //}
    static inline size_t _RoundUp(size_t bytes, size_t alignnum) {
        return ((bytes + alignnum - 1) & ~(alignnum - 1));
        /* explain: (alignnum - 1) 即为对齐数，按位取反后，需要对齐的部分变为0... 待研究！ */
    }
    static inline size_t RoundUp(size_t bytes) {
        if(bytes <= 128) {
            return _RoundUp(bytes, 8);
        } else if(bytes <= 1024) {
            return _RoundUp(bytes, 16);
        } else if(bytes <= 8*1024) {
            return _RoundUp(bytes, 128);
        } else if(bytes <= 64*1024) {
            return _RoundUp(bytes, 1024);
        } else if(bytes <= 256*1024) {
            return _RoundUp(bytes, 8*1024);
        } else {
            return _RoundUp(bytes, 1 << PAGE_SHIFT);
            assert(false);
        }
        return -1;
    }

    //static inline size_t _Index(size_t bytes, size_t alignnum) {
    //    if(bytes % alignnum == 0) return bytes / alignnum - 1;
    //    else return bytes / alignnum;
    //}
    static inline size_t _Index(size_t bytes, size_t align_shift) {
        return ((bytes + (1 << align_shift) - 1) >> align_shift) - 1;
        // 需要研究一下
    }
    static inline size_t Index(size_t bytes) {
        assert(bytes <= MAX_BYTES);
        // 每个区间有多少个链
        static int group_array[4] = {16, 56, 56, 56};
        if(bytes <= 128) {
            return _Index(bytes, 3);
        } else if(bytes <= 1024) {
            return _Index(bytes - 128, 4) + group_array[0];
        } else if(bytes <= 8*1024) {
            return _Index(bytes - 1024, 7) + group_array[1] + group_array[0];
        } else if(bytes <= 64*1024) {
            return _Index(bytes - 8*1024, 10) + group_array[2] + group_array[1] + group_array[0];
        } else if(bytes <= 256*1024) {
            return _Index(bytes - 64*1024, 13) + group_array[3] + group_array[2] + group_array[1] + group_array[0];
        } else {
            assert(false);
        }
        return -1;
    }

    /* brief: 一次threadcache从中心缓存获取多少个对象 */
    static size_t NumMoveSize(size_t size) {
        assert(size > 0);
        // [2, 512] 一次批量移动多少个对象的（慢启动）上限值
        // 小对象一次批量上限高
        // 大对象一次批量上限低
        int num = MAX_BYTES / size;
        if(num < 2) num = 2;
        if(num > 512) num = 512;

        return num;
    }

    /* brief: 一次centralcache从pagecache获取多少个span */
    static size_t NumMovePage(size_t size) {
        size_t num = NumMoveSize(size);
        size_t npage = num * size;

        npage >>= PAGE_SHIFT; //右移 13 位 即 除 8 * 1024
        if(npage == 0) npage = 1;

        return npage;
    }
};

/* brief: 管理多个连续页大块内存跨度结构 */
struct Span
{
    PAGE_ID _pageId = 0; // 大块内存起始页的页号
    size_t _page_num = 0;       // 页的数量

    Span *_next = nullptr;    //双向链表的结构
    Span *_prev = nullptr;

    size_t _obj_size = 0; // 切好的小对象的大小
    size_t _use_count = 0; // 切好小块内存，被分配给threadcache的计数
    void *_freelist = nullptr;   // 切好的小块内存的自由链表

    bool _isUse = false; //是否在被使用
};

/* brief: 带头双向循环链表 */
class SpanList
{
public:
    SpanList() {
        _head = new Span;
        _head->_next = _head;
        _head->_prev = _head;
    }

    Span *Begin() { return _head->_next; }
    Span *End() { return _head; }

    bool Empty() {
        return _head->_next == _head;
    }

    void Insert(Span *pos, Span *newSpan) {
        assert(pos);
        assert(newSpan);

        Span *prev = pos->_prev;

        prev->_next = newSpan;
        newSpan->_prev = prev;
        newSpan->_next = pos;
        pos->_prev = newSpan;
    }

    void PushFront(Span *span) {
        Insert(Begin(), span);
    }

    Span *PopFront() {
        Span *front = _head->_next;
        Erase(front);
        return front;
    }
    void Erase(Span *pos) {
        assert(pos);
        assert(pos != _head);
        if(pos != _head) {

        }

        Span *prev = pos->_prev;
        Span *next = pos->_next;

        prev->_next = next;
        next->_prev = prev;
    }
private:
    Span *_head;
public:
    std::mutex _mtx; //桶锁
};