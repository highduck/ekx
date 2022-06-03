#include <unit.h>

#include <ecx/ecx.hpp>
#include "common/components.hpp"

using namespace ecs;

SUITE(ecs) {
    IT("world, basic") {
        // TODO:
        ecx_setup();

        entity_t e[4] = {0, 1, 2, 3};
        e[1] = create_entity();
        CHECK_EQ(e[1].id, 1);
        CHECK_EQ(e[2].id, 2);
        CHECK_EQ(e[3].id, 3);

        ecx_shutdown();
    }

    IT("sparse_vector, basic") {
        ecx_setup();

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

        ecx_shutdown();
    }


    IT("view, each") {
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

    IT("view, locking") {
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

    IT("view, min_to_max") {
//    world& w = world::the;
//    uint32_t values_count = 0u;
//    for (uint32_t i = 0; i < 100; ++i) {
//        auto e = w.create();
//        w.add<position_t>(e);
//        if ((i & 1u) == 1u) {
//            w.add<motion_t>(e);
//        }
//        if ((i & 3u) == 3u) {
//            w.add<value_t>(e);
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


    IT("components, add") {
//        create_entity();
//        w.registerComponent<value_t>();
//        w.registerComponent<position_t>();
//        w.registerComponent<empty_comp_t>();
//
//        entity_t e;
//        w.create(&e, 1);
//        auto& v = w.add<value_t>(e);
//        REQUIRE_EQ(v.value, 0);
//
//        v.value = 10;
//        REQUIRE_EQ(10, v.value);
//        REQUIRE_EQ((w.get<value_t>(e).value), 10);
//
//
//        // check if comp pool is not created
//        REQUIRE_FALSE(w.has<empty_comp_t>(e));
//        REQUIRE_FALSE(w.has<position_t>(e));
//
//        // not registered component could not be checked
//        // ASSERT_FALSE(w.has<not_used_comp_t>(e));
//
//        // check ensure works
//        w.create(&e, 1);
//        w.add<position_t>(e);
//        REQUIRE(w.has<position_t>(e));
//
//        w.shutdown();
    }

    IT("components, remove") {
        ecx_setup();
        ECX_COMPONENT(value_t);

        entity_t e = create_entity();

        ecs::add<value_t>(e).value = 1;
        CHECK_EQ(ecs::get<value_t>(e).value, 1);
        CHECK(ecs::has<value_t>(e));
        ecs::remove<value_t>(e);
        CHECK_FALSE(ecs::has<value_t>(e));
        destroy_entity(e);

        ecx_shutdown();
    }

    IT("components, abstract_clear") {
        ecx_setup();
        ECX_COMPONENT(value_t);
        ECX_COMPONENT(position_t);

        entity_t e = create_entity();

        ecs::add<value_t>(e).value = 1;
        ecs::add<position_t>(e) = {1.0f, 1.0f};
        CHECK(ecs::has<value_t>(e));
        CHECK(ecs::has<position_t>(e));
        destroy_entity(e);
        CHECK_FALSE(is_entity(e));
        // will assert `e` is not valid entity
        //REQUIRE_FALSE(ecs::has<value_t>(e));
        //REQUIRE_FALSE(ecs::has<position_t>(e));

        ecx_shutdown();
    }

    constexpr Entity null{};
    constexpr Entity nullRef{};

    IT("entity_value, basic") {
        CHECK_EQ(null.idx, 0u);
        CHECK_EQ(nullRef.gen, 0u);
    }
}
