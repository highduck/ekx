#include <ecxx/impl/entity.hpp>

#include <gtest/gtest.h>

using namespace ecxx;

constexpr entity null{};

TEST(entity_value, basic) {
    ASSERT_EQ(null.index(), 0u);
    ASSERT_EQ(null.version(), 0u);

    entity e{1, 1};
    ASSERT_EQ(e.index(), 1u);
    ASSERT_EQ(e.version(), 1u);

    e.index(23);
    e.version(99);
    ASSERT_EQ(e.index(), 23u);
    ASSERT_EQ(e.version(), 99u);
}