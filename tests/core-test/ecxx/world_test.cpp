#include <ecxx/impl/world.hpp>
#include <gtest/gtest.h>

using namespace ecs;

TEST(world, basic) {
    // TODO:
    auto e = world::the.create();
    world::the.destroy(e);
    world::the.clear();
}
