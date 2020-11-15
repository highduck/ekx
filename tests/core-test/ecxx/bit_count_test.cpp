#include <ecxx/impl/utility.hpp>
#include <gtest/gtest.h>

using namespace ecs::details;

TEST(bit_count, func) {
    ASSERT_EQ(bit_count(0x0), 0);

    ASSERT_EQ(bit_count(0x1), 1);
    ASSERT_EQ(bit_count(0x2), 1);
    ASSERT_EQ(bit_count(0x40000), 1);
    ASSERT_EQ(bit_count(0x80000000), 1);

    ASSERT_EQ(bit_count(0x3), 2);
    ASSERT_EQ(bit_count(0x40001), 2);

    ASSERT_EQ(bit_count(0xF), 4);
    ASSERT_EQ(bit_count(0xFF), 8);
    ASSERT_EQ(bit_count(0xFFFF), 16);
    ASSERT_EQ(bit_count(0xFFFFFFFF), 32);
    ASSERT_EQ(bit_count(0xFFFFFFFFFFFFFFFF), 64);
}
