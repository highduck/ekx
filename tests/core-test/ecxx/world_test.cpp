#include <ecxx/impl/world.hpp>
#include <gtest/gtest.h>

using namespace ecxx;

TEST(world, basic) {
    // TODO:
    world w;
    auto e = w.create();
    w.destroy(e);
}
