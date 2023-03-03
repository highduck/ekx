#ifndef BMFONT_EXPORT_H
#define BMFONT_EXPORT_H

#include <ek/bitmap.h>

#ifdef __cplusplus
extern "C" {
#endif
//
//typedef struct {
////    Resolution resolutions[8];
//    uint32_t resolutionsNum = 0;
//} image_set_t;

typedef struct {
    uint32_t a;
    uint32_t b;
} codepoint_pair_t;

typedef enum {
    BITMAP_FILTER_NONE = 0,
    BITMAP_FILTER_REFILL = 1,
    BITMAP_FILTER_GLOW = 2,
    BITMAP_FILTER_SHADOW = 3
} bitmap_filter_type;


typedef struct {
    bitmap_filter_type type;
    float blurX;
    float blurY;
    float strength;
    int quality;
    vec4_t color0;
    vec4_t color1;

    // TODO: check we change it from radian to degree in serialization, we need change calculations for usages
    float angle;
    float distance;

    // refill options
    float top;
    float bottom;

    // flags
    bool knockout;
    bool innerGlow;
    bool hideObject;
} bitmap_filter_t;

typedef struct {
    const char* ttf_path;
    codepoint_pair_t ranges[32];
    uint32_t ranges_num;
    bitmap_filter_t filters[32];
    uint32_t filters_num;
    float resolutions[8];
    uint32_t resolutions_num;
    uint16_t font_size;
    bool mirror_case;
    bool use_kerning;
} bitmap_font_build_options_t;

typedef struct {
    uint32_t glyph_index;

    // source rectangle in base physical units
    rect_t rc;

    irect_t source;

    uint8_t padding;

    // reference image;
    bitmap_t bitmap;
} image_t;

typedef struct {
    uint32_t index;
    float scale;
    image_t* images;
} resolution_t;

typedef struct {
    resolution_t resolutions[8];
    uint32_t resolutions_num;
} image_set_t;

typedef struct {
    uint32_t* codepoints;
    rect_t box;
    float advance_x;
    uint32_t glyph_index;
} glyph_t;

typedef struct {
    float fontSize;
    float lineHeight;
    float ascender;
    float descender;
    glyph_t* glyphs;
} font_t;

bitmap_filter_type bitmap_filter_type_from_string(const char* name);
const char* bitmap_filter_type_to_string(bitmap_filter_type type);

void convert_a8_to_argb32pma(uint8_t const* source_a8_buf, uint32_t* dest_argb32_buf, int pixels_count);

void convert_a8_to_argb32(uint8_t const* source_a8_buf,
                          uint32_t* dest_argb32_buf,
                          int pixels_count);

rect_t combine_bounds(rect_t a, rect_t b);

bitmap_filter_t bitmap_filter(void);
void parse_bitmap_font_build_options(FILE* f, bitmap_font_build_options_t* options);
bitmap_filter_t bitmap_filter_scale(bitmap_filter_t filter, float scale);
rect_t extend_filter_rect(rect_t rect, const bitmap_filter_t* filter);
rect_t extend_blur_rect_filter(rect_t rect, const bitmap_filter_t* filter);
void extend_blur_rect(rect_t* rect, float blurX, float blurY, int pass);
ivec2_t round_blur_radius(float rx, float ry);
void apply_bitmap_filter(bitmap_t bitmap, const bitmap_filter_t* filter, irect_t bounds);
void bitmap_filters_copy_scale(const bitmap_filter_t* src, bitmap_filter_t* dest, uint32_t n, float scale);
void apply_bitmap_filters(const bitmap_filter_t* filters, uint32_t n, float scale, rect_t* rc, irect_t* source,
                          bitmap_t* bitmap);

void save_font_bitmaps(const char* font_name, const image_set_t* images, const char* output);


int export_bitmap_font(const char* config_path);

#ifdef __cplusplus
}
#endif

#endif // BMFONT_EXPORT_H
