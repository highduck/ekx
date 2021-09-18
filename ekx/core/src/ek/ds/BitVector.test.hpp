#include <doctest.h>

#include "BitVector.hpp"

TEST_CASE("bit_vector_test, basic") {
    using namespace ek;
    {
        BitVector vec{100};

        REQUIRE(vec.data() != nullptr);
        REQUIRE(vec.size() == 100);

        REQUIRE_EQ(vec[33], false);
        REQUIRE_EQ(vec[34], false);
        vec[91] = true;
        REQUIRE_EQ(vec[91], true);

        REQUIRE_EQ(vec.get(94), false);
        vec.set(94, true);
        REQUIRE_EQ(vec.get(94), true);

        REQUIRE_EQ(vec[99], false);
        vec.enable(99);
        REQUIRE_EQ(vec[99], true);
        REQUIRE_EQ(vec.is_false(99), false);
        vec.disable(99);
        REQUIRE_EQ(vec[99], false);
        REQUIRE_EQ(vec.is_false(99), true);

        vec.enable_if_not(99);
        REQUIRE_EQ(vec[99], true);
        vec.enable_if_not(99);
        REQUIRE_EQ(vec[99], true);
    }
}
