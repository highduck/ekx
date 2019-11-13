#include <ek/math/vec.hpp>

#include <gtest/gtest.h>

using namespace ek;

TEST(Vector, Operators) {

    float2 v2{1.0f, 1.0f};
    v2 *= 2.0f;

    ASSERT_FLOAT_EQ(v2.x, 2.0f);
    ASSERT_FLOAT_EQ(v2.y, 2.0f);

    float3 v3{1.0f, 1.0f, 1.0f};
    v3 *= 2.0f;

    ASSERT_FLOAT_EQ(v3.x, 2.0f);
    ASSERT_FLOAT_EQ(v3.y, 2.0f);
    ASSERT_FLOAT_EQ(v3.z, 2.0f);

    float4 v4{1.0f, 1.0f, 1.0f, 1.0f};
    v4 *= 2.0f;

    ASSERT_FLOAT_EQ(v4.x, 2.0f);
    ASSERT_FLOAT_EQ(v4.y, 2.0f);
    ASSERT_FLOAT_EQ(v4.z, 2.0f);
    ASSERT_FLOAT_EQ(v4.w, 2.0f);
}