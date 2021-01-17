#include <ecxx/impl/world.hpp>
#include <gtest/gtest.h>

using namespace ecs;

TEST(world, basic) {
    // TODO:
    world w;
    world_initialize(&w);

    Entity e[4];
    entity_create(&w, &e[1], 1);
    ASSERT_EQ(e[1], 1);
    ASSERT_EQ(e[2], 2);
    ASSERT_EQ(e[3], 3);

    world_shutdown(&w);
}
