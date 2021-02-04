#include <ecxx/impl/world.hpp>
#include <gtest/gtest.h>

using namespace ecs;

TEST(sparse_vector, basic) {
    world w;
    w.initialize();

    // TODO:
//    auto* eth = &w.maps[0];
//    ASSERT_EQ(sparse_array_get(eth, 2), 0);
//    ASSERT_EQ(sparse_array_get(eth, 3), 0);
//
//    sparse_array_insert(eth, 2, 1);
//    sparse_array_insert(eth, 4, 1);
//
//    ASSERT_EQ(sparse_array_get(eth, 0), 0);
//    ASSERT_EQ(sparse_array_get(eth, 1), 0);
//    ASSERT_EQ(sparse_array_get(eth, 2), 1);
//    ASSERT_EQ(sparse_array_get(eth, 3), 0);
//    ASSERT_EQ(sparse_array_get(eth, 4), 1);
//    ASSERT_EQ(sparse_array_get(eth, 5), 0);
//
//    sparse_array_remove(eth, 4);
//    ASSERT_EQ(sparse_array_get(eth, 0), 0);
//    ASSERT_EQ(sparse_array_get(eth, 1), 0);
//    ASSERT_EQ(sparse_array_get(eth, 2), 1);
//    ASSERT_EQ(sparse_array_get(eth, 3), 0);
//    ASSERT_EQ(sparse_array_get(eth, 4), 0);
//    ASSERT_EQ(sparse_array_get(eth, 5), 0);
//
//    sparse_array_remove(eth, 2);
//    ASSERT_EQ(sparse_array_get(eth, 0), 0);
//    ASSERT_EQ(sparse_array_get(eth, 1), 0);
//    ASSERT_EQ(sparse_array_get(eth, 2), 0);
//    ASSERT_EQ(sparse_array_get(eth, 3), 0);
//    ASSERT_EQ(sparse_array_get(eth, 4), 0);
//    ASSERT_EQ(sparse_array_get(eth, 5), 0);

    w.shutdown();
}
