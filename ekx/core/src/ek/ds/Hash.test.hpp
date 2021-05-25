#include <doctest.h>

#include "Hash.hpp"

TEST_CASE("Hash general") {
    using namespace ek;

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

TEST_CASE("Hash stress") {
    using namespace ek;

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