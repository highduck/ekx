#include <gtest/gtest.h>
#include <ek/imaging/image.hpp>
#include <ek/imaging/drawing.hpp>

using namespace ek;

TEST(image_test, create) {
    image_t img{16, 16};
    //4, 0xFFFFFFFF_argb};
    ASSERT_EQ(16 * 4, img.stride());
    ASSERT_EQ(0x0, img.row(0)->abgr);

    fill_image(img, 0xFFFFFFFFu);
    ASSERT_EQ(0xFFFFFFFF, img.row(0)->abgr);
}
