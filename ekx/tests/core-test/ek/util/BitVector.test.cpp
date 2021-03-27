#include <ek/ds/BitVector.hpp>
#include <gtest/gtest.h>
#include <ek/Allocator.hpp>

using namespace ek;

TEST(bit_vector_test, basic) {
    memory::initialize();
    {
        BitVector vec{100};

        ASSERT_TRUE(vec.data() != nullptr);
        ASSERT_TRUE(vec.size() == 100);

        ASSERT_EQ(vec[33], false);
        ASSERT_EQ(vec[34], false);
        vec[91] = true;
        ASSERT_EQ(vec[91], true);

        ASSERT_EQ(vec.get(94), false);
        vec.set(94, true);
        ASSERT_EQ(vec.get(94), true);

        ASSERT_EQ(vec[99], false);
        vec.enable(99);
        ASSERT_EQ(vec[99], true);
        ASSERT_EQ(vec.is_false(99), false);
        vec.disable(99);
        ASSERT_EQ(vec[99], false);
        ASSERT_EQ(vec.is_false(99), true);

        vec.enable_if_not(99);
        ASSERT_EQ(vec[99], true);
        vec.enable_if_not(99);
        ASSERT_EQ(vec[99], true);
    }
    memory::shutdown();
}
