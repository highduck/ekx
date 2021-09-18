#include <doctest.h>

#include <ecxx/ecxx.hpp>
#include "common/components.hpp"

using namespace ecs;
using namespace ek;

TEST_CASE("world, basic") {
    // TODO:
    World w;
    w.initialize();

    EntityIndex e[4] = {0, 1, 2, 3};
    w.create(&e[1], 1);
    REQUIRE_EQ(e[1], 1);
    REQUIRE_EQ(e[2], 2);
    REQUIRE_EQ(e[3], 3);

    w.shutdown();
}

TEST_CASE("sparse_vector, basic") {
    World w;
    w.initialize();

    // TODO:
//    auto* eth = &w.maps[0];
//    REQUIRE_EQ(sparse_array_get(eth, 2), 0);
//    REQUIRE_EQ(sparse_array_get(eth, 3), 0);
//
//    sparse_array_insert(eth, 2, 1);
//    sparse_array_insert(eth, 4, 1);
//
//    REQUIRE_EQ(sparse_array_get(eth, 0), 0);
//    REQUIRE_EQ(sparse_array_get(eth, 1), 0);
//    REQUIRE_EQ(sparse_array_get(eth, 2), 1);
//    REQUIRE_EQ(sparse_array_get(eth, 3), 0);
//    REQUIRE_EQ(sparse_array_get(eth, 4), 1);
//    REQUIRE_EQ(sparse_array_get(eth, 5), 0);
//
//    sparse_array_remove(eth, 4);
//    REQUIRE_EQ(sparse_array_get(eth, 0), 0);
//    REQUIRE_EQ(sparse_array_get(eth, 1), 0);
//    REQUIRE_EQ(sparse_array_get(eth, 2), 1);
//    REQUIRE_EQ(sparse_array_get(eth, 3), 0);
//    REQUIRE_EQ(sparse_array_get(eth, 4), 0);
//    REQUIRE_EQ(sparse_array_get(eth, 5), 0);
//
//    sparse_array_remove(eth, 2);
//    REQUIRE_EQ(sparse_array_get(eth, 0), 0);
//    REQUIRE_EQ(sparse_array_get(eth, 1), 0);
//    REQUIRE_EQ(sparse_array_get(eth, 2), 0);
//    REQUIRE_EQ(sparse_array_get(eth, 3), 0);
//    REQUIRE_EQ(sparse_array_get(eth, 4), 0);
//    REQUIRE_EQ(sparse_array_get(eth, 5), 0);

    w.shutdown();
}


TEST_CASE("view, each") {
//    world& w = world::the;
//    w.create<position_t, motion_t>();
//    int i = 0;
//    view<position_t, motion_t>().each([&i](auto& pos, auto& mot) {
//        ++i;
//    });
//    REQUIRE_EQ(i, 1);
//
//    w.clear();
}

TEST_CASE("view, locking") {
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
//    REQUIRE_EQ(view_count, 5 + 1);
//
//    w.clear();
}

TEST_CASE("view, min_to_max") {
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
//    REQUIRE_EQ(view_count, values_count);
//    w.clear();
}


TEST_CASE("components, add") {
        World w;
        w.initialize();
        w.registerComponent<value_t>();
        w.registerComponent<position_t>();
        w.registerComponent<empty_comp_t>();

        EntityIndex e;
        w.create(&e, 1);
        auto& v = w.assign<value_t>(e);
        REQUIRE_EQ(v.value, 0);

        v.value = 10;
        REQUIRE_EQ(10, v.value);
        REQUIRE_EQ((w.get<value_t>(e).value), 10);


        // check if comp pool is not created
        REQUIRE_FALSE(w.has<empty_comp_t>(e));
        REQUIRE_FALSE(w.has<position_t>(e));

        // not registered component could not be checked
        // ASSERT_FALSE(w.has<not_used_comp_t>(e));

        // check ensure works
        w.create(&e, 1);
        w.assign<position_t>(e);
        REQUIRE(w.has<position_t>(e));

        w.shutdown();
}

TEST_CASE("components, remove") {
    World w;
    w.initialize();
    w.registerComponent<value_t>();

    EntityIndex e;
    w.create(&e, 1);

    w.assign<value_t>(e, 1);
    REQUIRE_EQ(w.get<value_t>(e).value, 1);
    REQUIRE(w.has<value_t>(e));
    w.remove<value_t>(e);
    REQUIRE_FALSE(w.has<value_t>(e));
    w.destroy(&e, 1);

    w.shutdown();
}

TEST_CASE("components, abstract_clear") {
    World w;
    w.initialize();
    w.registerComponent<value_t>();
    w.registerComponent<position_t>();

    EntityIndex e;
    w.create(&e, 1);

    w.assign<value_t>(e, 1);
    w.assign<position_t>(e, 1.0f, 1.0f);
    REQUIRE(w.has<value_t>(e));
    REQUIRE(w.has<position_t>(e));
    w.destroy(&e, 1);
    REQUIRE_FALSE(w.isAllocated(e));
    REQUIRE_FALSE(w.has<value_t>(e));
    REQUIRE_FALSE(w.has<position_t>(e));

    w.shutdown();
}

constexpr EntityApi null{};
constexpr EntityRef nullRef{};

TEST_CASE("entity_value, basic") {
    REQUIRE_EQ(null.index, 0u);
    REQUIRE_EQ(nullRef.version(), 0u);

    EntityRef e{1, 1};
    REQUIRE_EQ(e.index(), 1u);
    REQUIRE_EQ(e.version(), 1u);

    e.index(23);
    e.version(99);
    REQUIRE_EQ(e.index(), 23u);
    REQUIRE_EQ(e.version(), 99u);
}