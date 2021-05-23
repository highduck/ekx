#include <doctest.h>

#include <ek/Allocator.hpp>
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
TEST_CASE("Allocators, base") {
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

struct NonTrivialStruct {
    int counter = 0;

    NonTrivialStruct() {
        ++counter;
    }

    // issue `crash on Array::push_back with std::string`
    NonTrivialStruct& operator=(const NonTrivialStruct& o) {
        // check if object is alive (valid state)
        // this case brake if data structure use COPY ASSIGNMENT
        // on invalid state object after destructor was called
        EK_ASSERT(counter > 0);
        counter = o.counter;
        return *this;
    }

    ~NonTrivialStruct() {
        --counter;
        EK_ASSERT(counter == 0);
    }
};

TEST_CASE("Array, moveCopy") {
    memory::initialize();

    {
        Array<int> a;
        a.push_back(1);
        Array<int> movedA{std::move(a)};
        movedA.push_back(3);

        REQUIRE(movedA[0] == 1);
        REQUIRE(movedA[1] == 3);
    }
    {
        Array<NonTrivialStruct> a;
        a.push_back(NonTrivialStruct());
        a.eraseAt(0u);
        a.push_back(NonTrivialStruct());
    }
    {
        Array<int> a;
        a.push_back(1);
        Array<int> movedA = std::move(a);
        movedA.push_back(3);

        REQUIRE(movedA[0] == 1);
        REQUIRE(movedA[1] == 3);
    }

    {
        Array<int> a;
        a.push_back(1);
        Array<int> copyA{a};
        copyA.push_back(3);
        a.push_back(2);

        REQUIRE(copyA[0] == 1);
        REQUIRE(copyA[1] == 3);
        REQUIRE(a[0] == 1);
        REQUIRE(a[1] == 2);
    }
    {
        Array<int> a;
        a.push_back(1);
        Array<int> copyA = a;
        copyA.push_back(3);
        a.push_back(2);

        REQUIRE(copyA[0] == 1);
        REQUIRE(copyA[1] == 3);
        REQUIRE(a[0] == 1);
        REQUIRE(a[1] == 2);
    }

    memory::shutdown();
}

TEST_CASE("Hash, general") {
    memory::initialize();
    {
        Hash<int> h{};
        REQUIRE(h.get(0, 99) == 99);
        REQUIRE(!h.has(0));
        h.remove(0);
        h.set(1000, 123);
        REQUIRE(h.get(1000, 0) == 123);
        REQUIRE(h.get(2000, 99) == 99);

        for (int i = 0; i < 100; ++i) {
            h.set(i, i * i);
        }
        for (int i = 0; i < 100; ++i) {
            REQUIRE(h.get(i, 0) == i * i);
        }
        h.remove(1000);
        REQUIRE(!h.has(1000));
        h.remove(2000);
        REQUIRE(h.get(1000, 0) == 0);
        for (int i = 0; i < 100; ++i) {
            REQUIRE(h.get(i, 0) == i * i);
        }
        h.clear();
        for (int i = 0; i < 100; ++i) {
            REQUIRE(!h.has(i));
        }
    }
    memory::shutdown();
}

TEST_CASE("Hash, stress") {
    memory::initialize();
    {
        const size_t i0 = 0xFFFFFFFF00000000;
        //const size_t i1 = i0 + 10000000;
        const size_t i1 = i0 + 1000;
        Hash<int> h{};
        for(size_t i = i0; i < i1; ++i) {
            REQUIRE(!h.has(i));
            REQUIRE(h.get(i, 0) == 0);
            h.set(i, 1);
            REQUIRE(h.has(i));
            REQUIRE(h.get(i, 0) == 1);
        }
        for(size_t i = i0; i < i1; ++i) {
            REQUIRE(h.has(i));
            REQUIRE(h.get(i, 0) == 1);
            h.remove(i);
            REQUIRE(!h.has(i));
            REQUIRE(h.get(i, 0) == 0);
        }
        for(size_t i = i0; i < i1; ++i) {
            REQUIRE(!h.has(i));
            REQUIRE(h.get(i, 0) == 0);
            h.set(i, 1);
            REQUIRE(h.has(i));
            REQUIRE(h.get(i, 0) == 1);
        }
        for(size_t i = i0; i < i1; ++i) {
            REQUIRE(h.has(i));
            REQUIRE(h.get(i, 0) == 1);
            h.remove(i);
            REQUIRE(!h.has(i));
            REQUIRE(h.get(i, 0) == 0);
        }
        h.clear();
    }
    memory::shutdown();
}