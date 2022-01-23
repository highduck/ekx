#include <ek/bitmap.h>
#include <stdlib.h>

#define CHECK_EQ(x, y) do{ \
if((x) != (y)) { puts("FAILED: CHECK_EQ(" #x ", " #y ")\n"); abort(); } \
}while(0)

int main(int argc, char** argv) {
    uint32_t pixel;
    bitmap_t bmp = {1, 1, (color_t*)&pixel};

    /// RGBA <-> BGRA
    pixel = 0xFF112233u;
    bitmap_swizzle_xwzy(&bmp);
    CHECK_EQ(pixel, 0xFF332211u);

    pixel = 0x0u;
    bitmap_swizzle_xwzy(&bmp);
    CHECK_EQ(pixel, 0x0u);

    pixel = 0xFFAA00AAu;
    bitmap_swizzle_xwzy(&bmp);
    CHECK_EQ(pixel, 0xFFAA00AAu);

    /// premultiply
    pixel = 0xFF112233u;
    bitmap_premultiply(&bmp);
    CHECK_EQ(pixel, 0xFF112233u);

    pixel = 0x00FFEEDDu;
    bitmap_premultiply(&bmp);
    CHECK_EQ(pixel, 0x00000000u);

    pixel = 0x77FFFFFF;
    bitmap_premultiply(&bmp);
    CHECK_EQ(pixel, 0x77777777u);

    /// un-premultiply
    pixel = 0xFF112233u;
    bitmap_unpremultiply(&bmp);
    CHECK_EQ(pixel, 0xFF112233u);

    pixel = 0x00000000u;
    bitmap_unpremultiply(&bmp);
    CHECK_EQ(pixel, 0x00000000u);

    pixel = 0x77777777;
    bitmap_unpremultiply(&bmp);
    CHECK_EQ(pixel, 0x77FFFFFFu);

    /// fill
    uint32_t pixels[4] = {};
    bitmap_t bmp2x2 = {2, 2, (color_t*)pixels};

    CHECK_EQ(pixels[0], 0u);
    CHECK_EQ(pixels[1], 0u);
    CHECK_EQ(pixels[2], 0u);
    CHECK_EQ(pixels[3], 0u);

    bitmap_fill(&bmp2x2, color_u32(0xDEADBABEu));

    CHECK_EQ(pixels[0], 0xDEADBABEu);
    CHECK_EQ(pixels[1], 0xDEADBABEu);
    CHECK_EQ(pixels[2], 0xDEADBABEu);
    CHECK_EQ(pixels[3], 0xDEADBABEu);

    return 0;
}
