#include <ek/Allocator.hpp>
#include <gtest/gtest.h>
#include <ek/ds/Array.hpp>
#include <ek/ds/Hash.hpp>

using namespace ek;

class A {
public:
    float a;
    float b;
    uint32_t count;
};

// check initialization / shutdown
// allocate and deallocate
// create and destroy proxy
TEST(Allocators, base) {
    memory::initialize();

    Allocator& a = memory::stdAllocator;

    void* p = a.alloc(100, sizeof(void*));
//    ASSERT(a.allocated_size(p) >= 100);
//    ASSERT(a.total_allocated() >= 100);
    void* q = a.alloc(100, sizeof(void*));
//    ASSERT(a.allocated_size(q) >= 100);
//    ASSERT(a.total_allocated() >= 200);

    {
        ProxyAllocator proxy{a, "scope"};
        void* r = proxy.alloc(10, 4);
        proxy.dealloc(r);
    }

    {
        Allocator* proxy = a.create<ProxyAllocator>(a, "scope");
        void* r = proxy->alloc(10, 4);
        proxy->dealloc(r);
        a.destroy(proxy);
    }

    a.dealloc(p);
    a.dealloc(q);

    {
        // aligning issue
        void* ptr = a.alloc(1028, 8);
        // should be 1032, not 1024
        a.dealloc(ptr);
    }

    memory::shutdown();
}

TEST(Array, moveCopy) {
    memory::initialize();

    {
        Array<int> a;
        a.push_back(1);
        Array<int> movedA{std::move(a)};
        movedA.push_back(3);

        EK_ASSERT(movedA[0] == 1);
        EK_ASSERT(movedA[1] == 3);
    }
    {
        Array<int> a;
        a.push_back(1);
        Array<int> movedA = std::move(a);
        movedA.push_back(3);

        EK_ASSERT(movedA[0] == 1);
        EK_ASSERT(movedA[1] == 3);
    }

    {
        Array<int> a;
        a.push_back(1);
        Array<int> copyA{a};
        copyA.push_back(3);
        a.push_back(2);

        EK_ASSERT(copyA[0] == 1);
        EK_ASSERT(copyA[1] == 3);
        EK_ASSERT(a[0] == 1);
        EK_ASSERT(a[1] == 2);
    }
    {
        Array<int> a;
        a.push_back(1);
        Array<int> copyA = a;
        copyA.push_back(3);
        a.push_back(2);

        EK_ASSERT(copyA[0] == 1);
        EK_ASSERT(copyA[1] == 3);
        EK_ASSERT(a[0] == 1);
        EK_ASSERT(a[1] == 2);
    }

    memory::shutdown();
}

TEST(Hash, general) {
    memory::initialize();
    {
        Hash<int> h{};
        EK_ASSERT(h.get(0, 99) == 99);
        EK_ASSERT(!h.has(0));
        h.remove(0);
        h.set(1000, 123);
        EK_ASSERT(h.get(1000, 0) == 123);
        EK_ASSERT(h.get(2000, 99) == 99);

        for (int i = 0; i < 100; ++i) {
            h.set(i, i * i);
        }
        for (int i = 0; i < 100; ++i) {
            EK_ASSERT(h.get(i, 0) == i * i);
        }
        h.remove(1000);
        EK_ASSERT(!h.has(1000));
        h.remove(2000);
        EK_ASSERT(h.get(1000, 0) == 0);
        for (int i = 0; i < 100; ++i) {
            EK_ASSERT(h.get(i, 0) == i * i);
        }
        h.clear();
        for (int i = 0; i < 100; ++i) {
            EK_ASSERT(!h.has(i));
        }
    }
    memory::shutdown();
}