#ifndef EK_MATH_COLOR_H
#define EK_MATH_COLOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef union color_t {
    struct {
        uint8_t r, g, b, a;
    };
    uint32_t value;
    uint8_t data[4];
#ifdef __cplusplus

    inline uint8_t& operator[](const int index) { return data[index]; }

#endif
} color_t;

typedef struct color2_t {
    struct {
        color_t scale;
        color_t offset;
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

#ifdef __cplusplus
#define ARGB(x) (color_t{{(uint8_t)(((x) >> 16u) & 0xFFu),(uint8_t)(((x) >> 8u) & 0xFFu),(uint8_t)((x) & 0xFFu),(uint8_t)((x) >> 24u)}})
#define RGB(x) (color_t{{(uint8_t)(((x) >> 16u) & 0xFFu),(uint8_t)(((x) >> 8u) & 0xFFu),(uint8_t)((x) & 0xFFu),0xFFu}})
#else
#define ARGB(x) ((color_t){{(uint8_t)(((x) >> 16u) & 0xFFu),(uint8_t)(((x) >> 8u) & 0xFFu),(uint8_t)((x) & 0xFFu),(uint8_t)((x) >> 24u)}})
#define RGB(x) ((color_t){{(uint8_t)(((x) >> 16u) & 0xFFu),(uint8_t)(((x) >> 8u) & 0xFFu),(uint8_t)((x) & 0xFFu),0xFFu}})
#endif
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
color2f_t color2f_tint(color_t color, float intensity);
color2_t color2_tint(color_t color, uint8_t intensity);

color2f_t lerp_color2f(color2f_t a, color2f_t b, float t);
color2f_t mul_color2f(color2f_t a, color2f_t b);
vec4_t color2f_transform(color2f_t mod, vec4_t color);

color_t color_u32(uint32_t rgba);
color_t color_4f(float r, float g, float b, float a);
color_t color_4u(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
color_t color_vec4(vec4_t colorf);
color_t color_hue(float hue_unorm);
vec4_t colorf_hue(float hue_unorm);
color_t color_hsv(vec4_t hsv);
vec4_t hsv_from_color(color_t color);

color_t mul_color(color_t color, color_t multiplier);
color_t scale_color(color_t color, uint8_t multiplier);
color_t add_color(color_t color, color_t add);
color_t lerp_color(color_t a, color_t b, float t);
color_t color_alpha_scale_f(color_t color, float alpha_multiplier);

color2_t color2_identity(void);
color_t color2_get_offset(color_t base_scale, color_t offset);
void color2_add(color2_t* color, color_t offset);
void color2_concat(color2_t* color, color_t scale, color_t offset);
void color2_mul(color2_t* out, color2_t l, color2_t r);

vec4_t vec4_color(color_t color);

#ifdef __cplusplus
}
#endif

#include "color.hpp"

#endif //EK_MATH_COLOR_H
