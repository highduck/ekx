#include <ecxx/impl/world.hpp>
#include <gtest/gtest.h>

using namespace ecs;

TEST(world, basic) {
    // TODO:
    world w;
    w.initialize();

    Entity e[4] = {0, 1, 2, 3};
    w.create(&e[1], 1);
    ASSERT_EQ(e[1], 1);
    ASSERT_EQ(e[2], 2);
    ASSERT_EQ(e[3], 3);

    w.shutdown();
}
