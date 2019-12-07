#include <ek/math/packed_color.hpp>

#include <gtest/gtest.h>
#include <iostream>

using namespace ek;

TEST(PackedColor, BRG) {

    auto red = 0xFF0000_rgb;
    auto blue = 0x0000FF_rgb;
    auto yellow = 0xFFFF00_rgb;

    ASSERT_EQ(red.argb, 0xFFFF0000u);
    ASSERT_EQ(blue.argb, 0xFF0000FFu);
    ASSERT_EQ(yellow.argb, 0xFFFFFF00u);

    auto red_bgr = red.bgr();
    auto blue_bgr = blue.bgr();
    auto yellow_bgr = yellow.bgr();

    ASSERT_EQ(red_bgr.abgr, 0x0000FFu);
    ASSERT_EQ(blue_bgr.abgr, 0xFF0000u);
    ASSERT_EQ(yellow_bgr.abgr, 0x00FFFFu);
}