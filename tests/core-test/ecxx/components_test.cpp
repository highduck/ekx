#include <ecxx/ecxx.hpp>

#include <gtest/gtest.h>
#include "common/components.hpp"

using namespace ecxx;

TEST(components, add) {
    world world;

    auto e = world.create();
    auto& v = world.assign<value_t>(e);
    ASSERT_EQ(v.value, 0);

    v.value = 10;
    ASSERT_EQ(10, v.value);
    ASSERT_EQ((world.get<value_t>(e).value), 10);

    // const access
    const auto& cw = world;
    ASSERT_EQ((cw.get<value_t>(e).value), 10);

    // check if comp pool is not created
    ASSERT_FALSE(world.has<empty_comp_t>(e));
    ASSERT_FALSE(world.has<position_t>(e));

    // check ensure works
    e = world.create();
    world.assign<position_t>(e);
    ASSERT_TRUE(world.has<position_t>(e));
}

TEST(components, remove) {
    world world;
    auto e = world.create();
    world.assign<value_t>(e, 1);
    ASSERT_EQ(world.get<value_t>(e).value, 1);
    ASSERT_TRUE(world.has<value_t>(e));
    world.remove<value_t>(e);
    ASSERT_FALSE(world.has<value_t>(e));
    world.destroy(e);
}

TEST(components, abstract_clear) {
    world world;
    auto e = world.create();
    world.assign<value_t>(e, 1);
    world.assign<position_t>(e, 1.0f, 1.0f);
    ASSERT_TRUE(world.has<value_t>(e));
    ASSERT_TRUE(world.has<position_t>(e));
    world.destroy(e);
    ASSERT_FALSE(world.valid(e));
    ASSERT_FALSE(world.has<value_t>(e));
    ASSERT_FALSE(world.has<position_t>(e));
}