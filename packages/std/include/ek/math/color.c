#ifdef __cplusplus
extern "C" {
#endif

rgba_t rgba_u32(uint32_t value) {
    return (rgba_t) {.value = value};
}

rgba_t rgba_4f(const float r, const float g, const float b, const float a) {
    EK_ASSERT_R2(r >= 0.0f && r <= 1.0f);
    EK_ASSERT_R2(g >= 0.0f && g <= 1.0f);
    EK_ASSERT_R2(b >= 0.0f && b <= 1.0f);
    EK_ASSERT_R2(a >= 0.0f && a <= 1.0f);
    return (rgba_t) {
            .r = (uint8_t) (r * 255.0f),
            .g = (uint8_t) (g * 255.0f),
            .b = (uint8_t) (b * 255.0f),
            .a = (uint8_t) (a * 255.0f),
    };
}

rgba_t rgba_vec4(const vec4_t rgba) {
    return rgba_4f(rgba.x, rgba.y, rgba.z, rgba.w);
}

rgba_t rgba_mul(const rgba_t color, const rgba_t multiplier) {
    return (rgba_t) {
            .r = u8_norm_mul(color.r, multiplier.r),
            .g = u8_norm_mul(color.g, multiplier.g),
            .b = u8_norm_mul(color.b, multiplier.b),
            .a = u8_norm_mul(color.a, multiplier.a)
    };
}

rgba_t rgba_scale(const rgba_t color, const uint8_t multiplier) {
    return (rgba_t) {
            .r = u8_norm_mul(color.r, multiplier),
            .g = u8_norm_mul(color.g, multiplier),
            .b = u8_norm_mul(color.b, multiplier),
            .a = u8_norm_mul(color.a, multiplier)
    };
}

rgba_t rgba_add(const rgba_t color, const rgba_t add) {
    return (rgba_t) {
            .r = u8_add_sat(color.r, add.r),
            .g = u8_add_sat(color.g, add.g),
            .b = u8_add_sat(color.b, add.b),
            .a = u8_add_sat(color.a, add.a)
    };
}

rgba_t rgba_lerp(const rgba_t a, const rgba_t b, const float t) {
    const uint32_t r = (uint32_t) (t * 1024);
    const uint32_t ri = 1024u - r;
    return (rgba_t) {
            .r = (uint8_t) ((ri * a.r + r * b.r) >> 10u),
            .g = (uint8_t) ((ri * a.g + r * b.g) >> 10u),
            .b = (uint8_t) ((ri * a.b + r * b.b) >> 10u),
            .a = (uint8_t) ((ri * a.a + r * b.a) >> 10u)
    };
}

rgba_t rgba_alpha_scale_f(rgba_t color, const float alpha_multiplier) {
    color.a = (uint8_t) ((float) color.a * alpha_multiplier);
    return color;
}

color2_t color2_identity() {
    color2_t result;
    result.scale.value = 0xFFFFFFFFu;
    result.offset.value = 0;
    return result;
}

rgba_t color2_get_offset(rgba_t base_scale, rgba_t offset) {
    return (rgba_t) {
            .r = u8_norm_mul(offset.r, base_scale.r),
            .g = u8_norm_mul(offset.g, base_scale.g),
            .b = u8_norm_mul(offset.b, base_scale.b),
            .a = offset.a
    };
}

void color2_add(color2_t* color, rgba_t offset) {
    if (offset.value != 0) {
        color->offset = rgba_add(color->offset, color2_get_offset(color->scale, offset));
    }
}

void color2_concat(color2_t* color, rgba_t scale, rgba_t offset) {
    if (offset.value != 0) {
        color->offset = rgba_add(color->offset, color2_get_offset(color->scale, offset));
    }
    if (scale.value != 0xFFFFFFFF) {
        color->scale = rgba_mul(color->scale, scale);
    }
}

void color2_mul(color2_t* out, color2_t l, color2_t r) {
    out->scale = (~r.scale.value) != 0 ? rgba_mul(l.scale, r.scale) : l.scale;
    out->offset = r.offset.value != 0 ? rgba_add(l.offset, color2_get_offset(l.scale, r.offset)) : l.offset;
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

color2f_t color2f_tint(rgba_t color, float intensity) {
    const float inv_i = 1.0f - intensity;
    const float i2b = intensity / 255.0f;
    color2f_t result;
    result.scale = vec4(
            inv_i,
            inv_i,
            inv_i,
            (float) color.a * 255.0f
    );
    result.offset = vec4(
            (float) color.r * i2b,
            (float) color.g * i2b,
            (float) color.b * i2b,
            0
    );
    return result;
}

color2f_t lerp_color2f(color2f_t a, color2f_t b, float t) {
    color2f_t result;
    result.scale = vec4_lerp(a.scale, b.scale, t);
    result.offset = vec4_lerp(a.offset, b.offset, t);
    return result;
}

color2f_t mul_color2f(color2f_t a, color2f_t b) {
    color2f_t result;
    result.scale = vec4_mul(a.scale, b.scale);
    result.offset = vec4_add(a.offset, vec4_mul(a.scale, b.offset));
    return result;
}

vec4_t color2f_transform(color2f_t mod, vec4_t color) {
    return vec4_add(vec4_mul(color, mod.scale), mod.offset);
}

vec4_t vec4_rgba(const rgba_t rgba) {
    vec4_t v;
    v.x = (float) rgba.r / 255.0f;
    v.y = (float) rgba.g / 255.0f;
    v.z = (float) rgba.b / 255.0f;
    v.w = (float) rgba.a / 255.0f;
    return v;
}

#ifdef __cplusplus
}
#endif
