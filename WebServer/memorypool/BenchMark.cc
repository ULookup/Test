#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <atomic>
#include <chrono>
#include "ConcurrentAlloc.h"

using namespace std;

static const int THREADS = 8;
static const int OPS_PER_THREAD = 500000;

// 用于对比 malloc/free
static void* (*alloc_fn)(size_t) = nullptr;
static void (*free_fn)(void*) = nullptr;

/*******************************************************
 * 真实业务风格测试：模式 1
 * Burst Alloc → Burst Free （典型缓存池命中场景）
 *******************************************************/
void test_burst_alloc_free() {
    vector<void*> vec;
    vec.reserve(OPS_PER_THREAD);

    // 批量分配
    for(int i = 0; i < OPS_PER_THREAD; i++)
        vec.push_back(alloc_fn(24));

    // 批量释放
    for(void* p : vec)
        free_fn(p);
}

/*******************************************************
 * 真实业务风格测试：模式 2
 * 随机生命周期（retain 20% 对象）
 *******************************************************/
void test_mixed_retain() {
    vector<void*> vec;
    vec.reserve(OPS_PER_THREAD);

    std::mt19937 rng(std::hash<std::thread::id>()(std::this_thread::get_id()) ^ time(NULL));
    std::uniform_int_distribution<int> dist(0, 99);

    // 随机保留 20%
    for(int i = 0; i < OPS_PER_THREAD; i++) {
        void* p = alloc_fn(24);
        if(dist(rng) < 80) free_fn(p);   // 80% 立即释放
        else vec.push_back(p);           // 20% 保留
    }

    // 剩余释放
    for(void* p : vec)
        free_fn(p);
}

/*******************************************************
 * 真实业务风格测试：模式 3
 * 随机大小（4~256 bytes），多 size class 测试
 *******************************************************/
void test_random_size() {
    std::mt19937 rng(std::hash<std::thread::id>()(std::this_thread::get_id()) ^ time(NULL));
    std::uniform_int_distribution<int> dist(4, 256);

    vector<void*> vec;
    vec.reserve(OPS_PER_THREAD);

    for(int i = 0; i < OPS_PER_THREAD; i++) {
        size_t sz = dist(rng);
        void* p = alloc_fn(sz);
        vec.push_back(p);
    }

    for(void* p : vec)
        free_fn(p);
}

/*******************************************************
 * 统一 worker 封装
 *******************************************************/
void worker(int mode) {
    switch(mode) {
        case 1: test_burst_alloc_free(); break;
        case 2: test_mixed_retain();     break;
        case 3: test_random_size();      break;
    }
}

////////////////////////////////////////////////////////
// 主测试函数
////////////////////////////////////////////////////////
void run_test(const char* name, int mode,
              void* (*alloc_func)(size_t),
              void (*free_func)(void*)) 
{
    alloc_fn = alloc_func;
    free_fn = free_func;

    cout << "\n==== Running Test: " << name << " ====\n";

    auto start = chrono::high_resolution_clock::now();

    vector<thread> tlist;
    for(int i = 0; i < THREADS; i++)
        tlist.emplace_back(worker, mode);

    for(auto& t : tlist)
        t.join();

    auto end = chrono::high_resolution_clock::now();
    double ms = chrono::duration<double, milli>(end - start).count();

    long long total_ops = THREADS * OPS_PER_THREAD;
    cout << "Total ops = " << total_ops << endl;
    cout << "Time = " << ms << " ms" << endl;
    cout << "Ops/sec = " << (total_ops / (ms / 1000.0)) / 1e6 << " M ops/s\n";
}

////////////////////////////////////////////////////////
// main：对比 malloc vs 内存池
////////////////////////////////////////////////////////
int main() {
    // 测试 1：Burst
    run_test("Burst Alloc/Free    (IceMemoryPool)", 
             1, ConcurrentAlloc, ConcurrentFree);
    run_test("Burst Alloc/Free    (malloc/free)", 
             1, 
             [](size_t s){ return malloc(s); },
             [](void* p){ free(p); });

    // 测试 2：随机生命周期
    run_test("Mixed Retain/Free   (IceMemoryPool)", 
             2, ConcurrentAlloc, ConcurrentFree);
    run_test("Mixed Retain/Free   (malloc/free)", 
             2,
             [](size_t s){ return malloc(s); },
             [](void* p){ free(p); });

    // 测试 3：多 size class 随机
    run_test("Random Sizes        (IceMemoryPool)", 
             3, ConcurrentAlloc, ConcurrentFree);
    run_test("Random Sizes        (malloc/free)", 
             3,
             [](size_t s){ return malloc(s); },
             [](void* p){ free(p); });

    return 0;
}
