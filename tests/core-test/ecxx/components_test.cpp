#include <ecxx/ecxx.hpp>

#include <gtest/gtest.h>
#include "common/components.hpp"

using namespace ecs;

TEST(components, add) {
    world w;
    w.initialize();

    Entity e;
    w.create(&e, 1);
    auto& v = w.assign<value_t>(e);
    ASSERT_EQ(v.value, 0);

    v.value = 10;
    ASSERT_EQ(10, v.value);
    ASSERT_EQ((w.get<value_t>(e).value), 10);


    // check if comp pool is not created
    ASSERT_FALSE(w.has<empty_comp_t>(e));
    ASSERT_FALSE(w.has<position_t>(e));

    // check ensure works
    w.create(&e, 1);
    w.assign<position_t>(e);
    ASSERT_TRUE(w.has<position_t>(e));

    w.shutdown();
}

TEST(components, remove) {
    world w;
    w.initialize();

    Entity e;
    w.create(&e, 1);

    w.assign<value_t>(e, 1);
    ASSERT_EQ(w.get<value_t>(e).value, 1);
    ASSERT_TRUE(w.has<value_t>(e));
    w.remove<value_t>(e);
    ASSERT_FALSE(w.has<value_t>(e));
    w.destroy(&e, 1);

    w.shutdown();
}

TEST(components, abstract_clear) {
    world w;
    w.initialize();

    Entity e;
    w.create(&e, 1);

    w.assign<value_t>(e, 1);
    w.assign<position_t>(e, 1.0f, 1.0f);
    ASSERT_TRUE(w.has<value_t>(e));
    ASSERT_TRUE(w.has<position_t>(e));
    w.destroy(&e, 1);
    ASSERT_FALSE(w.isAllocated(e));
    ASSERT_FALSE(w.has<value_t>(e));
    ASSERT_FALSE(w.has<position_t>(e));

    w.shutdown();
}