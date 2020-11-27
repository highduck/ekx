#include <ecxx/impl/entity_map.hpp>
#include <gtest/gtest.h>

using namespace ecs;

template<typename T>
void test() {
    entity_map<T> m;

    m.emplace({1u, 0u}, 1);
    ASSERT_TRUE(m.has({1u, 0u}));
    ASSERT_FALSE(m.has({5u, 0u}));

    m.emplace({5u, 0u}, 1);
    ASSERT_TRUE(m.has({5u, 0u}));

    T def = m.get_or_default({2u, 0u});
    ASSERT_EQ(def, 0);

    T not_def = m.get_or_default({5u, 0u});
    ASSERT_EQ(not_def, 1);

    T created = m.get_or_create({2u, 0u});
    ASSERT_EQ(created, T{});
    ASSERT_TRUE(m.has({2u, 0u}));

    m.erase({2u, 0u});
    ASSERT_FALSE(m.has({2u, 0u}));

    // check `swap and pop`
    m.erase({1u, 0u});
    ASSERT_FALSE(m.has({1u, 0u}));
    ASSERT_TRUE(m.has({5u, 0u}));
}

TEST(entity_map, simple_has_data) {
    entity_map<int> m;

    m.emplace({1u, 0u}, 1);
    ASSERT_TRUE(m.dataTable.has(1u));
    ASSERT_FALSE(m.dataTable.has(5u));

    m.emplace({5u, 0u}, 1);
    ASSERT_TRUE(m.dataTable.has(5u));

    auto def = m.get_or_default(2u);
    ASSERT_EQ(def, 0);

    auto not_def = m.get_or_default(5u);
    ASSERT_EQ(not_def, 1);

    auto created = m.get_or_create({2u, 0u});
    ASSERT_EQ(created, int{});
    ASSERT_TRUE(m.dataTable.has(2u));

    auto already_created = m.get_or_create({2u, 0u});
    ASSERT_EQ(already_created, int{});

    m.erase(2u);
    ASSERT_FALSE(m.dataTable.has(2u));

    // check `swap and pop`
    m.erase(1u);
    ASSERT_FALSE(m.dataTable.has(1u));
    ASSERT_TRUE(m.dataTable.has(5u));
}

TEST(entity_map, simple_empty) {
    struct empty_t {

    };

    entity_map<empty_t> m;

    m.emplace({1u, 0u});
    ASSERT_TRUE(m.dataTable.has(1u));
    ASSERT_FALSE(m.dataTable.has(5u));

    m.emplace({5u, 0u});
    ASSERT_TRUE(m.dataTable.has(5u));

    auto def = m.get_or_default(2u);
    //ASSERT_EQ(def, v);

    auto not_def = m.get_or_default(5u);
    //ASSERT_EQ(not_def, v);

    auto created = m.get_or_create({2u, 0u});
    //ASSERT_EQ(created, v);
    ASSERT_TRUE(m.dataTable.has(2u));

    m.erase(2u);
    ASSERT_FALSE(m.dataTable.has(2u));

    // check `swap and pop`
    m.erase(1u);
    ASSERT_FALSE(m.dataTable.has(1u));
    ASSERT_TRUE(m.dataTable.has(5u));
}

TEST(entity_map, runtime_virtual) {
    struct empty_t {

    };

    entity_map<empty_t> map_without_data;
    entity_map<int> map_with_data;

    entity_map_base& m1 = map_without_data;
    m1.emplace_dyn({1u, 0u});
    m1.emplace_dyn({2u, 0u});
    m1.emplace_dyn({3u, 0u});
    ASSERT_EQ(m1.size(), 3);
    m1.erase_dyn(1u);
    m1.erase_dyn(3u);
    ASSERT_EQ(m1.size(), 1);

    entity_map_base& m2 = map_with_data;
    m2.emplace_dyn({1u, 0u});
    m2.emplace_dyn({2u, 0u});
    m2.emplace_dyn({3u, 0u});
    ASSERT_EQ(m2.size(), 3);
    m2.erase_dyn(1u);
    m2.erase_dyn(3u);
    ASSERT_EQ(m2.size(), 1);
}
