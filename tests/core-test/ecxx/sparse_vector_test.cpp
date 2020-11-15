#include <ecxx/impl/sparse_vector.hpp>
#include <gtest/gtest.h>

using namespace ecs;

TEST(sparse_vector, basic) {
    sparse_vector v;
    ASSERT_FALSE(v.has(2));
    ASSERT_FALSE(v.has(3, 3));

    v.insert(2, 1);
    v.insert(4, 1);

    ASSERT_TRUE(v.has(2));
    ASSERT_TRUE(v.has(4));

    ASSERT_EQ(v.at(2), 1);
    ASSERT_EQ(v.at(4), 1);

    v.remove(2);
    ASSERT_EQ(v.at(2), 0);
    ASSERT_FALSE(v.has(2));

    v.remove(4);
    ASSERT_EQ(v.at(4), 0);
    ASSERT_FALSE(v.has(4));
}
