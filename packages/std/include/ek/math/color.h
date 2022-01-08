#ifndef EK_MATH_COLOR_H
#define EK_MATH_COLOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef union rgba_t {
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
    uint32_t value;
    uint8_t data[4];
#ifdef __cplusplus

    inline uint8_t& operator[](const int index) { return data[index]; }

#endif
} rgba_t;

typedef struct color2_t {
    struct {
        rgba_t scale;
        rgba_t offset;
    };
    struct {
        uint8_t red_scale;
        uint8_t green_scale;
        uint8_t blue_scale;
        uint8_t alpha;
        uint8_t red_offset;
        uint8_t green_offset;
        uint8_t blue_offset;
        uint8_t additive;
    };
    uint64_t packed;
} color2_t;

typedef union color2f_t {
    struct {
        vec4_t scale;
        vec4_t offset;
    };
    struct {
        float red_scale;
        float green_scale;
        float blue_scale;
        float alpha;
        float red_offset;
        float green_offset;
        float blue_offset;
        float additive;
    };
} color2f_t;

#define COL32_SWAP_RB(x) (((x) & 0xFF00FF00u) | (((x) >> 16u) & 0xFFu) | (((x) & 0xFFu) << 16u))
#define ARGB(x) ((rgba_t){.value=(COL32_SWAP_RB((uint32_t)(x)))})
#define RGB(x) ((rgba_t){.value=(0xFF000000u | COL32_SWAP_RB((uint32_t)(x)))})
#define COLOR_WHITE ARGB(0xFFFFFFFF)
#define COLOR_BLACK ARGB(0xFF000000)
#define COLOR_ZERO ARGB(0x00000000)

color2f_t color2f(void);
color2f_t color2f_v(vec4_t scale, vec4_t offset);
/**
 *
 * @param color - RGBA part
 * @param intensity - float 0...1
 */
color2f_t color2f_tint(rgba_t color, float intensity);
color2f_t lerp_color2f(color2f_t a, color2f_t b, float t);
color2f_t mul_color2f(color2f_t a, color2f_t b);
vec4_t color2f_transform(color2f_t mod, vec4_t color);

rgba_t rgba_u32(uint32_t value);
rgba_t rgba_4f(float r, float g, float b, float a);
rgba_t rgba_vec4(vec4_t rgba);
rgba_t rgba_mul(rgba_t color, rgba_t multiplier);
rgba_t rgba_scale(rgba_t color, uint8_t multiplier);
rgba_t rgba_add(rgba_t color, rgba_t add);
rgba_t rgba_lerp(rgba_t a, rgba_t b, float t);
rgba_t rgba_alpha_scale_f(rgba_t color, float alpha_multiplier);

color2_t color2_identity();
rgba_t color2_get_offset(rgba_t base_scale, rgba_t offset);
void color2_add(color2_t* color, rgba_t offset);
void color2_concat(color2_t* color, rgba_t scale, rgba_t offset);
void color2_mul(color2_t* out, color2_t l, color2_t r);

vec4_t vec4_rgba(rgba_t rgba);

#ifdef __cplusplus
}
#endif

#include "color.hpp"

#endif //EK_MATH_COLOR_H
