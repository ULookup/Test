#pragma once
#include <atomic>
#include <cstdint>
#include <cstring>
#include "MetaPool.h"

// 36-bit pageId = 12 + 12 + 12
static const int PM_L3_BITS = 12;              // leaf slots = 4096
static const int PM_L2_BITS = 12;              // L2 node count = 4096
static const int PM_L1_BITS = 12;              // L1 root count = 4096

static const uint64_t PM_L1_SIZE = 1ULL << PM_L1_BITS; 
static const uint64_t PM_L2_SIZE = 1ULL << PM_L2_BITS; 
static const uint64_t PM_L3_SIZE = 1ULL << PM_L3_BITS;


// ---------------- Leaf -------------------
struct PageMapLeaf {
    std::atomic<void*> slots[PM_L3_SIZE];

    PageMapLeaf() {
        std::memset((void*)slots, 0, sizeof(slots));
    }
};

// --------------- Level 2 Node ----------------
struct PageMapL2 {
    std::atomic<PageMapLeaf*> leaf[PM_L2_SIZE];

    PageMapL2() {
        std::memset((void*)leaf, 0, sizeof(leaf));
    }
};

// ----------------- PageMap (3-level radix tree) -----------------
class PageMap {
public:
    std::atomic<PageMapL2*> root[PM_L1_SIZE];

    PageMap() {
        std::memset((void*)root, 0, sizeof(root));
    }

    // -------------------- SET -------------------------
    inline void set(uint64_t pageId, void* span) {
        const uint64_t i1 = pageId >> (PM_L2_BITS + PM_L3_BITS);            // high 12 bits
        const uint64_t i2 = (pageId >> PM_L3_BITS) & (PM_L2_SIZE - 1);      // mid 12 bits
        const uint64_t i3 = pageId & (PM_L3_SIZE - 1);                      // low 12 bits

        if (i1 >= PM_L1_SIZE) {
            // 超过 pageId 范围
            return;
        }

        // ----- level 1 -----
        PageMapL2* n2 = root[i1].load(std::memory_order_acquire);
        if (!n2) {
            PageMapL2* new_n2 = new PageMapL2();
            PageMapL2* expected = nullptr;

            if (!root[i1].compare_exchange_strong(
                    expected, new_n2,
                    std::memory_order_release,
                    std::memory_order_acquire)) {
                delete new_n2;
                n2 = expected;
            } else {
                n2 = new_n2;
            }
        }

        // ----- level 2 -----
        PageMapLeaf* leaf = n2->leaf[i2].load(std::memory_order_acquire);
        if (!leaf) {
            PageMapLeaf* newLeaf = new PageMapLeaf();
            PageMapLeaf* expected = nullptr;

            if (!n2->leaf[i2].compare_exchange_strong(
                    expected, newLeaf,
                    std::memory_order_release,
                    std::memory_order_acquire)) {
                delete newLeaf;
                leaf = expected;
            } else {
                leaf = newLeaf;
            }
        }

        // ----- leaf -----
        leaf->slots[i3].store(span, std::memory_order_release);
    }

    // -------------------- GET -------------------------
    inline void* get(uint64_t pageId) const {
        const uint64_t i1 = pageId >> (PM_L2_BITS + PM_L3_BITS);
        const uint64_t i2 = (pageId >> PM_L3_BITS) & (PM_L2_SIZE - 1);
        const uint64_t i3 = pageId & (PM_L3_SIZE - 1);

        if (i1 >= PM_L1_SIZE) {
            return nullptr;
        }

        PageMapL2* n2 = root[i1].load(std::memory_order_acquire);
        if (!n2) return nullptr;

        PageMapLeaf* leaf = n2->leaf[i2].load(std::memory_order_acquire);
        if (!leaf) return nullptr;

        return leaf->slots[i3].load(std::memory_order_acquire);
    }
};

