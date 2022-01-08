#include <doctest.h>

#include <ek/math/Color32.hpp>

TEST_SUITE_BEGIN("c++ packed color");

TEST_CASE("BGR") {

    using namespace ek;
    auto red = 0xFF0000_rgb;
    auto blue = 0x0000FF_rgb;
    auto yellow = 0xFFFF00_rgb;

    REQUIRE_EQ(red.argb, 0xFFFF0000u);
    REQUIRE_EQ(blue.argb, 0xFF0000FFu);
    REQUIRE_EQ(yellow.argb, 0xFFFFFF00u);

    rgba_t red_bgr = red;
    rgba_t blue_bgr = blue;
    rgba_t yellow_bgr = yellow;

    REQUIRE_EQ(red_bgr.value, 0x0000FFu);
    REQUIRE_EQ(blue_bgr.value, 0xFF0000u);
    REQUIRE_EQ(yellow_bgr.value, 0x00FFFFu);
}

TEST_SUITE_END();