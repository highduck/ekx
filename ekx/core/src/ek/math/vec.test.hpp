#include <doctest.h>

#include <ek/math/vec.hpp>

using namespace ek;

TEST_CASE("Vector Operators") {

    float2 v2{1.0f, 1.0f};
    v2 *= 2.0f;

    REQUIRE_EQ(v2.x, doctest::Approx(2.0f));
    REQUIRE_EQ(v2.y, doctest::Approx(2.0f));

    float3 v3{1.0f, 1.0f, 1.0f};
    v3 *= 2.0f;

    REQUIRE_EQ(v3.x, doctest::Approx(2.0f));
    REQUIRE_EQ(v3.y, doctest::Approx(2.0f));
    REQUIRE_EQ(v3.z, doctest::Approx(2.0f));

    float4 v4{1.0f, 1.0f, 1.0f, 1.0f};
    v4 *= 2.0f;

    REQUIRE_EQ(v4.x, doctest::Approx(2.0f));
    REQUIRE_EQ(v4.y, doctest::Approx(2.0f));
    REQUIRE_EQ(v4.z, doctest::Approx(2.0f));
    REQUIRE_EQ(v4.w, doctest::Approx(2.0f));
}