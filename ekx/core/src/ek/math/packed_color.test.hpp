#include <doctest.h>

#include <ek/math/packed_color.hpp>

using namespace ek;

TEST_CASE("PackedColor BRG") {

    auto red = 0xFF0000_rgb;
    auto blue = 0x0000FF_rgb;
    auto yellow = 0xFFFF00_rgb;

    REQUIRE_EQ(red.argb, 0xFFFF0000u);
    REQUIRE_EQ(blue.argb, 0xFF0000FFu);
    REQUIRE_EQ(yellow.argb, 0xFFFFFF00u);

    auto red_bgr = red.bgr();
    auto blue_bgr = blue.bgr();
    auto yellow_bgr = yellow.bgr();

    REQUIRE_EQ(red_bgr.abgr, 0x0000FFu);
    REQUIRE_EQ(blue_bgr.abgr, 0xFF0000u);
    REQUIRE_EQ(yellow_bgr.abgr, 0x00FFFFu);
}