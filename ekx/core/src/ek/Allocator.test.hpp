#include <doctest.h>

#include "Allocator.hpp"

// check initialization / shutdown
// allocate and deallocate
// create and destroy proxy
TEST_CASE("Allocators base") {
    using namespace ek;

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