#pragma once

#include <doctest.h>

#include <ek/math/Vec.hpp>

TEST_SUITE_BEGIN("c++ vector math");

TEST_CASE("Vector Operators") {
    using namespace ek;

    Vec2f v2{1.0f, 1.0f};
    v2 *= 2.0f;

    REQUIRE_EQ(v2.x, doctest::Approx(2.0f));
    REQUIRE_EQ(v2.y, doctest::Approx(2.0f));

    Vec3f v3{1.0f, 1.0f, 1.0f};
    v3 *= 2.0f;

    REQUIRE_EQ(v3.x, doctest::Approx(2.0f));
    REQUIRE_EQ(v3.y, doctest::Approx(2.0f));
    REQUIRE_EQ(v3.z, doctest::Approx(2.0f));

    Vec4f v4{1.0f, 1.0f, 1.0f, 1.0f};
    v4 *= 2.0f;

    REQUIRE_EQ(v4.x, doctest::Approx(2.0f));
    REQUIRE_EQ(v4.y, doctest::Approx(2.0f));
    REQUIRE_EQ(v4.z, doctest::Approx(2.0f));
    REQUIRE_EQ(v4.w, doctest::Approx(2.0f));
}

TEST_SUITE_END();