#ifdef __cplusplus
extern "C" {
#endif

color_t color_u32(uint32_t rgba) {
    return (color_t) {.value = rgba};
}

color_t color_4f(const float r, const float g, const float b, const float a) {
    EK_ASSERT_R2(r >= 0.0f && r <= 1.0f);
    EK_ASSERT_R2(g >= 0.0f && g <= 1.0f);
    EK_ASSERT_R2(b >= 0.0f && b <= 1.0f);
    EK_ASSERT_R2(a >= 0.0f && a <= 1.0f);
    return (color_t) {
            .r = (uint8_t) (r * 255.0f),
            .g = (uint8_t) (g * 255.0f),
            .b = (uint8_t) (b * 255.0f),
            .a = (uint8_t) (a * 255.0f),
    };
}

color_t color_4u(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return (color_t) {
            .r = r,
            .g = g,
            .b = b,
            .a = a,
    };
}

color_t color_vec4(const vec4_t colorf) {
    return color_4f(colorf.x, colorf.y, colorf.z, colorf.w);
}

color_t color_hue(float hue_unorm) {
    static const color_t table[7] = {
            RGB(0xFF0000),
            RGB(0xFFFF00),
            RGB(0x00FF00),
            RGB(0x00FFFF),
            RGB(0x0000FF),
            RGB(0xFF00FF),
            RGB(0xFF0000),
    };
    const float t = 6 * saturate(hue_unorm);
    const int index = (int)t;
    return lerp_color(table[index], table[index + 1], t - (float) index);
}

vec4_t colorf_hue(float hue_unorm) {
    static const vec4_t table[7] = {
            (vec4_t){{1,0,0,1}},
            (vec4_t){{1,1,0,1}},
            (vec4_t){{0,1,0,1}},
            (vec4_t){{0,1,1,1}},
            (vec4_t){{0,0,1,1}},
            (vec4_t){{1,0,1,1}},
            (vec4_t){{1,0,0,1}},
    };
    const float t = 6 * saturate(hue_unorm);
    const int index = (int)t;
    return lerp_vec4(table[index], table[index + 1], t - (float) index);
}

static float hsv_lerp_channel(float value, float x, float y) {
    // y + x * y * (value - 1.0f);
    return lerp_f32(1.0f, value, x) * y;
}

static float hsv_calc_hue(float max, float delta, float r, float g, float b) {
    float hue;
    if (r >= max) {
        hue = (g - b) / delta;
    } else if (g >= max) {
        hue = 2.0f + (b - r) / delta;
    } else {
        hue = 4.0f + (r - g) / delta;
    }

    hue /= (float)6;
    if (hue < 0.0f) {
        hue += 1.0f;
    }

    return hue;
}

color_t color_hsv(vec4_t hsv) {
    const vec4_t hue_color = colorf_hue(hsv.hue);
    color_t result;
    result.r = unorm8_f32(hsv_lerp_channel(hue_color.r, hsv.saturation, hsv.value));
    result.g = unorm8_f32(hsv_lerp_channel(hue_color.g, hsv.saturation, hsv.value));
    result.b = unorm8_f32(hsv_lerp_channel(hue_color.b, hsv.saturation, hsv.value));
    result.a = (uint8_t)(hsv.a * 255.0f);
    return result;
}

vec4_t hsv_from_color(color_t color) {
    vec4_t result;
    const uint8_t r = color.r;
    const uint8_t g = color.g;
    const uint8_t b = color.b;
    const float min = (float)(MIN(r, MIN(g, b)));
    const float max = (float)(MAX(r, MAX(g, b)));
    const float delta = max - min;
    result.value = max / 255.0f;
    if (max > 0.0f && delta > 0.0f) {
        result.saturation = delta / max;
        result.hue = hsv_calc_hue(max, delta, r, g, b);
    } else {
        result.saturation = 0.0f;
        result.hue = -1.0f;
    }
    result.alpha = (float)color.a / 255.0f;
    return result;
}

color_t mul_color(color_t color, color_t multiplier) {
    return (color_t) {
            .r = u8_norm_mul(color.r, multiplier.r),
            .g = u8_norm_mul(color.g, multiplier.g),
            .b = u8_norm_mul(color.b, multiplier.b),
            .a = u8_norm_mul(color.a, multiplier.a)
    };
}

color_t scale_color(color_t color, uint8_t multiplier) {
    return (color_t) {
            .r = u8_norm_mul(color.r, multiplier),
            .g = u8_norm_mul(color.g, multiplier),
            .b = u8_norm_mul(color.b, multiplier),
            .a = u8_norm_mul(color.a, multiplier)
    };
}

color_t add_color(color_t color, color_t add) {
    return (color_t) {
            .r = u8_add_sat(color.r, add.r),
            .g = u8_add_sat(color.g, add.g),
            .b = u8_add_sat(color.b, add.b),
            .a = u8_add_sat(color.a, add.a)
    };
}

color_t lerp_color(color_t a, color_t b, float t) {
    const uint32_t r = (uint32_t) (t * 1024);
    const uint32_t ri = 1024u - r;
    return (color_t) {
            .r = (uint8_t) ((ri * a.r + r * b.r) >> 10u),
            .g = (uint8_t) ((ri * a.g + r * b.g) >> 10u),
            .b = (uint8_t) ((ri * a.b + r * b.b) >> 10u),
            .a = (uint8_t) ((ri * a.a + r * b.a) >> 10u)
    };
}

color_t color_alpha_scale_f(color_t color, const float alpha_multiplier) {
    color.a = (uint8_t) ((float) color.a * alpha_multiplier);
    return color;
}

color2_t color2_identity() {
    color2_t result;
    result.scale.value = 0xFFFFFFFFu;
    result.offset.value = 0;
    return result;
}

color_t color2_get_offset(color_t base_scale, color_t offset) {
    return (color_t) {
            .r = u8_norm_mul(offset.r, base_scale.r),
            .g = u8_norm_mul(offset.g, base_scale.g),
            .b = u8_norm_mul(offset.b, base_scale.b),
            .a = offset.a
    };
}

void color2_add(color2_t* color, color_t offset) {
    if (offset.value != 0) {
        color->offset = add_color(color->offset, color2_get_offset(color->scale, offset));
    }
}

void color2_concat(color2_t* color, color_t scale, color_t offset) {
    if (offset.value != 0) {
        color->offset = add_color(color->offset, color2_get_offset(color->scale, offset));
    }
    if (scale.value != 0xFFFFFFFF) {
        color->scale = mul_color(color->scale, scale);
    }
}

void color2_mul(color2_t* out, color2_t l, color2_t r) {
    out->scale = (~r.scale.value) != 0 ? mul_color(l.scale, r.scale) : l.scale;
    out->offset = r.offset.value != 0 ? add_color(l.offset, color2_get_offset(l.scale, r.offset)) : l.offset;
}

color2f_t color2f(void) {
    return color2f_v(vec4(1, 1, 1, 1), (vec4_t) {});
}

color2f_t color2f_v(vec4_t scale, vec4_t offset) {
    const color2f_t result = {
            .scale = scale,
            .offset = offset
    };
    return result;
}

color2f_t color2f_tint(color_t color, float intensity) {
    const float inv_i = 1.0f - intensity;
    const float i2b = intensity / 255.0f;
    color2f_t result;
    result.scale = vec4(
            inv_i,
            inv_i,
            inv_i,
            (float) color.a / 255.0f
    );
    result.offset = vec4(
            (float) color.r * i2b,
            (float) color.g * i2b,
            (float) color.b * i2b,
            0
    );
    return result;
}

color2_t color2_tint(color_t color, uint8_t intensity) {
    const uint8_t inv_i = 255u - intensity;
    color2_t result;
    result.scale = color_4u(
            inv_i,
            inv_i,
            inv_i,
            color.a
    );
    result.offset = color_4u(
            u8_norm_mul(color.r, intensity),
            u8_norm_mul(color.g, intensity),
            u8_norm_mul(color.b, intensity),
            0
    );
    return result;
}

color2f_t lerp_color2f(color2f_t a, color2f_t b, float t) {
    color2f_t result;
    result.scale = lerp_vec4(a.scale, b.scale, t);
    result.offset = lerp_vec4(a.offset, b.offset, t);
    return result;
}

color2f_t mul_color2f(color2f_t a, color2f_t b) {
    color2f_t result;
    result.scale = mul_vec4(a.scale, b.scale);
    result.offset = add_vec4(a.offset, mul_vec4(a.scale, b.offset));
    return result;
}

vec4_t color2f_transform(color2f_t mod, vec4_t color) {
    return add_vec4(mul_vec4(color, mod.scale), mod.offset);
}

vec4_t vec4_color(const color_t color) {
    vec4_t v;
    v.x = (float) color.r / 255.0f;
    v.y = (float) color.g / 255.0f;
    v.z = (float) color.b / 255.0f;
    v.w = (float) color.a / 255.0f;
    return v;
}

#ifdef __cplusplus
}
#endif
