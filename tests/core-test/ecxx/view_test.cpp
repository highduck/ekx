#include <ecxx/ecxx.hpp>
#include <gtest/gtest.h>
#include "common/components.hpp"

using namespace ecxx;

TEST(view, each) {
    world w;
    w.create<position_t, motion_t>();
    int i = 0;
    w.view<position_t, motion_t>().each([&i](auto& pos, auto& mot) {
        ++i;
    });
    ASSERT_EQ(i, 1);
}

TEST(view, locking) {
    world w;
    w.create<position_t, motion_t, value_t>();
    w.create<position_t, motion_t, value_t>();
    w.create<position_t, motion_t, value_t>();
    w.create<position_t, motion_t, value_t>();
    w.create<position_t, motion_t, value_t>();
    w.create<empty_comp_t>();

    ASSERT_FALSE(w.is_locked<position_t>());
    ASSERT_FALSE(w.is_locked<motion_t>());
    ASSERT_FALSE(w.is_locked<value_t>());
    ASSERT_FALSE(w.is_locked<empty_comp_t>());

    uint32_t view_count = 0u;
    {
        w.view<position_t, motion_t, value_t>().each([&view_count, &w](auto&& ...args) {
            ++view_count;
            ASSERT_TRUE(w.is_locked<position_t>());
            ASSERT_TRUE(w.is_locked<motion_t>());
            ASSERT_TRUE(w.is_locked<value_t>());
            ASSERT_FALSE(w.is_locked<empty_comp_t>());
        });
    }

    for (auto e : w.view<position_t, empty_comp_t>()) {
        // should not run (no entity found)
        ++view_count;
    }

    for (auto e : w.view<empty_comp_t>()) {
        ++view_count;
        ASSERT_FALSE(w.is_locked<position_t>());
        ASSERT_FALSE(w.is_locked<motion_t>());
        ASSERT_FALSE(w.is_locked<value_t>());
        ASSERT_TRUE(w.is_locked<empty_comp_t>());
    }

    ASSERT_FALSE(w.is_locked<position_t>());
    ASSERT_FALSE(w.is_locked<motion_t>());
    ASSERT_FALSE(w.is_locked<value_t>());
    ASSERT_FALSE(w.is_locked<empty_comp_t>());

    ASSERT_EQ(view_count, 5 + 1);
}


TEST(view, min_to_max) {
    world w;
    uint32_t values_count = 0u;
    for (uint32_t i = 0; i < 100; ++i) {
        auto e = w.create();
        w.assign<position_t>(e);
        if ((i & 1u) == 1u) {
            w.assign<motion_t>(e);
        }
        if ((i & 3u) == 3u) {
            w.assign<value_t>(e);
            values_count++;
        }
    }

    uint32_t view_count = 0u;
    w.view<position_t, motion_t, value_t>().each([&view_count](auto&& ...args) {
        ++view_count;
    });

    ASSERT_EQ(view_count, values_count);
}

TEST(runtime_view, min_to_max) {
    world w;
    uint32_t values_count = 0u;
    for (uint32_t i = 0; i < 100; ++i) {
        auto e = w.create();
        w.assign<position_t>(e);
        if ((i & 1u) == 1u) {
            w.assign<motion_t>(e);
        }
        if ((i & 3u) == 3u) {
            w.assign<value_t>(e);
            values_count++;
        }
    }

    uint32_t view_count = 0u;
    world::component_typeid types[] = {
            w.type<position_t>(),
            w.type<motion_t>(),
            w.type<value_t>()
    };
    w.runtime_view(std::begin(types), std::end(types)).each([&view_count](auto e) {
        ++view_count;
    });

    // FIXME: there is a bug when we have zero tables for iterator: need rewrite runtime view
    // no entities query
//    world_t::component_typeid types1[] = {
//            w.type<empty_comp_t>()
//    };
//
//    for (auto e : w.runtime_view(std::begin(types1), std::end(types1))) {
//        // should not run (no entity found)
//        ++view_count;
//    }

    // no match
    w.create<empty_comp_t>();
    w.create<empty_comp_t>();

    world::component_typeid types2[] = {
            w.type<position_t>(),
            w.type<empty_comp_t>()
    };

    for (auto e : w.runtime_view(std::begin(types2), std::end(types2))) {
        // should not run (no entity found)
        ++view_count;
    }

    ASSERT_EQ(view_count, values_count);
}
