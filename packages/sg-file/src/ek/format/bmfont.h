#ifndef EK_FORMAT_BMFONT_H
#define EK_FORMAT_BMFONT_H

#include <ek/math.h>
#include <ek/hash.h>

struct bmfont_glyph {
    rect_t box;
    float advance_x;
    string_hash_t sprite;
};

struct bmfont_header {
    // all metrics are in EM (units / units_per_EM)
    // base font size used to generate bitmaps
    float base_font_size;
    float line_height_multiplier;
    float ascender;
    float descender;
    uint32_t codepoints_data_offset;
    uint32_t codepoints_num;
    uint32_t glyphs_data_offset;
    uint32_t glyphs_num;
};

struct bmfont_file {
    bmfont_header* header;

    // (codepoint: u32 | glyph_index: u32)
    uint32_t* codepoints;

    bmfont_glyph* glyphs;
};

inline static void bmfont_file_map(const void* data, bmfont_file* out) {
    bmfont_header* hdr = (bmfont_header*)data;
    out->header = hdr;
    out->codepoints = (uint32_t*)((uint8_t*)data + hdr->codepoints_data_offset);
    out->glyphs = (bmfont_glyph*)((uint8_t*)data + hdr->glyphs_data_offset);
}

#endif // EK_FORMAT_BMFONT_H
