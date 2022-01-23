#pragma once

#include <ek/bitmap.h>

namespace bmfont_export {

void convert_a8_to_argb32pma(uint8_t const* source_a8_buf, uint32_t* dest_argb32_buf, int pixels_count);

void convert_a8_to_argb32(uint8_t const* source_a8_buf,
                          uint32_t* dest_argb32_buf,
                          int pixels_count);
}
