#include <gtest/gtest.h>

#include <ecxx/ecxx.hpp>
#include "common/components.hpp"
#include <ek/Allocator.hpp>

using namespace ecs;
using namespace ek;

TEST(world, basic) {
    memory::initialize();
    // TODO:
    World w;
    w.initialize();

    EntityIndex e[4] = {0, 1, 2, 3};
    w.create(&e[1], 1);
    ASSERT_EQ(e[1], 1);
    ASSERT_EQ(e[2], 2);
    ASSERT_EQ(e[3], 3);

    w.shutdown();

    memory::shutdown();
}

TEST(sparse_vector, basic) {
    memory::initialize();

    World w;
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
    memory::shutdown();
}


TEST(view, each) {
//    world& w = world::the;
//    w.create<position_t, motion_t>();
//    int i = 0;
//    view<position_t, motion_t>().each([&i](auto& pos, auto& mot) {
//        ++i;
//    });
//    ASSERT_EQ(i, 1);
//
//    w.clear();
}

TEST(view, locking) {
//    world& w = world::the;
//    w.create<position_t, motion_t, value_t>();
//    w.create<position_t, motion_t, value_t>();
//    w.create<position_t, motion_t, value_t>();
//    w.create<position_t, motion_t, value_t>();
//    w.create<position_t, motion_t, value_t>();
//    w.create<empty_comp_t>();
//
//    ASSERT_FALSE(w.is_locked<position_t>());
//    ASSERT_FALSE(w.is_locked<motion_t>());
//    ASSERT_FALSE(w.is_locked<value_t>());
//    ASSERT_FALSE(w.is_locked<empty_comp_t>());
//
//    uint32_t view_count = 0u;
//    {
//        view<position_t, motion_t, value_t>().each([&view_count, &w](auto&& ...args) {
//            ++view_count;
//            ASSERT_TRUE(w.is_locked<position_t>());
//            ASSERT_TRUE(w.is_locked<motion_t>());
//            ASSERT_TRUE(w.is_locked<value_t>());
//            ASSERT_FALSE(w.is_locked<empty_comp_t>());
//        });
//    }
//
//    for (auto e : view<position_t, empty_comp_t>()) {
//        // should not run (no entity found)
//        ++view_count;
//    }
//
//    for (auto e : view<empty_comp_t>()) {
//        ++view_count;
//        ASSERT_FALSE(w.is_locked<position_t>());
//        ASSERT_FALSE(w.is_locked<motion_t>());
//        ASSERT_FALSE(w.is_locked<value_t>());
//        ASSERT_TRUE(w.is_locked<empty_comp_t>());
//    }
//
//    ASSERT_FALSE(w.is_locked<position_t>());
//    ASSERT_FALSE(w.is_locked<motion_t>());
//    ASSERT_FALSE(w.is_locked<value_t>());
//    ASSERT_FALSE(w.is_locked<empty_comp_t>());
//
//    ASSERT_EQ(view_count, 5 + 1);
//
//    w.clear();
}

TEST(view, min_to_max) {
//    world& w = world::the;
//    uint32_t values_count = 0u;
//    for (uint32_t i = 0; i < 100; ++i) {
//        auto e = w.create();
//        w.assign<position_t>(e);
//        if ((i & 1u) == 1u) {
//            w.assign<motion_t>(e);
//        }
//        if ((i & 3u) == 3u) {
//            w.assign<value_t>(e);
//            values_count++;
//        }
//    }
//
//    uint32_t view_count = 0u;
//    view<position_t, motion_t, value_t>().each([&view_count](auto&& ...args) {
//        ++view_count;
//    });
//
//    ASSERT_EQ(view_count, values_count);
//    w.clear();
}


TEST(components, add) {
    memory::initialize();
    {
        World w;
        w.initialize();
        w.registerComponent<value_t>();
        w.registerComponent<position_t>();
        w.registerComponent<empty_comp_t>();

        EntityIndex e;
        w.create(&e, 1);
        auto& v = w.assign<value_t>(e);
        ASSERT_EQ(v.value, 0);

        v.value = 10;
        ASSERT_EQ(10, v.value);
        ASSERT_EQ((w.get<value_t>(e).value), 10);


        // check if comp pool is not created
        ASSERT_FALSE(w.has<empty_comp_t>(e));
        ASSERT_FALSE(w.has<position_t>(e));

        // not registered component could not be checked
        // ASSERT_FALSE(w.has<not_used_comp_t>(e));

        // check ensure works
        w.create(&e, 1);
        w.assign<position_t>(e);
        ASSERT_TRUE(w.has<position_t>(e));

        w.shutdown();
    }
    memory::shutdown();
}

TEST(components, remove) {
    memory::initialize();
    World w;
    w.initialize();
    w.registerComponent<value_t>();

    EntityIndex e;
    w.create(&e, 1);

    w.assign<value_t>(e, 1);
    ASSERT_EQ(w.get<value_t>(e).value, 1);
    ASSERT_TRUE(w.has<value_t>(e));
    w.remove<value_t>(e);
    ASSERT_FALSE(w.has<value_t>(e));
    w.destroy(&e, 1);

    w.shutdown();
    memory::shutdown();
}

TEST(components, abstract_clear) {
    memory::initialize();
    World w;
    w.initialize();
    w.registerComponent<value_t>();
    w.registerComponent<position_t>();

    EntityIndex e;
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
    memory::shutdown();
}

constexpr EntityApi null{};
constexpr EntityRef nullRef{};

TEST(entity_value, basic) {
    ASSERT_EQ(null.index, 0u);
    ASSERT_EQ(nullRef.version(), 0u);

    EntityRef e{1, 1};
    ASSERT_EQ(e.index(), 1u);
    ASSERT_EQ(e.version(), 1u);

    e.index(23);
    e.version(99);
    ASSERT_EQ(e.index(), 23u);
    ASSERT_EQ(e.version(), 99u);
}