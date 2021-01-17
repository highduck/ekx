#include <ecxx/ecxx.hpp>

#include <gtest/gtest.h>
#include "common/components.hpp"

using namespace ecs;

TEST(components, add) {
    world w;
    world_initialize(&w);

    Entity e;
    entity_create(&w, &e, 1);
    auto& v = entity_assign<value_t>(&w, e);
    ASSERT_EQ(v.value, 0);

    v.value = 10;
    ASSERT_EQ(10, v.value);
    ASSERT_EQ((entity_get<value_t>(&w, e).value), 10);


    // check if comp pool is not created
    ASSERT_FALSE(entity_has<empty_comp_t>(&w, e));
    ASSERT_FALSE(entity_has<position_t>(&w, e));

    // check ensure works
    entity_create(&w, &e, 1);
    entity_assign<position_t>(&w, e);
    ASSERT_TRUE(entity_has<position_t>(&w, e));

    world_shutdown(&w);
}

TEST(components, remove) {
    world w;
    world_initialize(&w);

    Entity e;
    entity_create(&w, &e, 1);

    entity_assign<value_t>(&w, e, 1);
    ASSERT_EQ(entity_get<value_t>(&w, e).value, 1);
    ASSERT_TRUE(entity_has<value_t>(&w, e));
    entity_remove<value_t>(&w, e);
    ASSERT_FALSE(entity_has<value_t>(&w, e));
    entity_destroy(&w, &e, 1);

    world_shutdown(&w);
}

TEST(components, abstract_clear) {
    world w;
    world_initialize(&w);

    Entity e;
    entity_create(&w, &e, 1);

    entity_assign<value_t>(&w, e, 1);
    entity_assign<position_t>(&w, e, 1.0f, 1.0f);
    ASSERT_TRUE(entity_has<value_t>(&w, e));
    ASSERT_TRUE(entity_has<position_t>(&w, e));
    entity_destroy(&w, &e, 1);
    ASSERT_FALSE(entity_is_allocated(&w, e));
    ASSERT_FALSE(entity_has<value_t>(&w, e));
    ASSERT_FALSE(entity_has<position_t>(&w, e));

    world_shutdown(&w);
}