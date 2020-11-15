#include <ecxx/ecxx.hpp>

#include <gtest/gtest.h>
#include "common/components.hpp"

using namespace ecs;

TEST(components, add) {
    world& world = world::the;

    auto e = world.create();
    auto& v = world.assign<value_t>(e);
    ASSERT_EQ(v.value, 0);

    v.value = 10;
    ASSERT_EQ(10, v.value);
    ASSERT_EQ((get<value_t>(e).value), 10);

    // const access
    const auto& cw = world::the;
    ASSERT_EQ((get<value_t>(e).value), 10);

    // check if comp pool is not created
    ASSERT_FALSE(has<empty_comp_t>(e));
    ASSERT_FALSE(has<position_t>(e));

    // check ensure works
    e = world.create();
    world.assign<position_t>(e);
    ASSERT_TRUE(has<position_t>(e));

    world.clear();
}

TEST(components, remove) {
    world& world = world::the;

    auto e = world.create();
    world.assign<value_t>(e, 1);
    ASSERT_EQ(get<value_t>(e).value, 1);
    ASSERT_TRUE(has<value_t>(e));
    remove<value_t>(e);
    ASSERT_FALSE(has<value_t>(e));
    world.destroy(e);

    world.clear();
}

TEST(components, abstract_clear) {
    world& world = world::the;

    auto e = world.create();
    world.assign<value_t>(e, 1);
    world.assign<position_t>(e, 1.0f, 1.0f);
    ASSERT_TRUE(e.has<value_t>());
    ASSERT_TRUE(e.has<position_t>());
    world.destroy(e);
    ASSERT_FALSE(valid(e));
    ASSERT_FALSE(e.has<value_t>());
    ASSERT_FALSE(e.has<position_t>());

    world.clear();
}