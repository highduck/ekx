
#ifdef __cplusplus
extern "C" {
#endif


/** scalar utilities **/
float to_radians(float degrees) {
    return degrees * MATH_PI / 180.0f;
}

float to_degrees(float radians) {
    return radians * 180.0f / MATH_PI;
}

/*
 * Thanks to good old Bit Twiddling Hacks for this one:
 * http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
 */
uint32_t next_power_of_2(uint32_t x) {
    EK_ASSERT(x <= 0x40000000u);
    x--;
    x |= x >> 1u;
    x |= x >> 2u;
    x |= x >> 4u;
    x |= x >> 8u;
    x |= x >> 16u;
    x++;
    return x;
}

// the difference of regular approach is rounding error compensation,
// which bring much better results when for cases such as `-326.400024 == -326.399994`
// ref: http://realtimecollisiondetection.net/blog/?t=89
bool almost_eq_f32(float a, float b, float eps) {
    // trivial impl:
    // return fabsf(a - b) <= eps;
    EK_ASSERT(eps >= 0.0f);
    const float lhs = fabsf(a - b);
    const float aa = fabsf(a);
    const float ab = fabsf(b);
    const float rhs = eps * MAX(1.0f, MAX(aa, ab));
    return lhs <= rhs;
}

float f32_lerp(float a, float b, float t) {
    return (1.0f - t) * a + t * b;
}

/** u8 functions **/

uint8_t unorm8_f32(float f) {
    EK_ASSERT(f >= 0.0f && f <= 1.0f);
    return (uint8_t) (f * 255.0f);
}

uint8_t unorm8_f32_clamped(float f) {
    if (UNLIKELY(f < 0.0f)) {
        return 0;
    } else if (UNLIKELY(f > 1.0f)) {
        return 255;
    }
    return (uint8_t) (f * 255.0f);
}

uint8_t u8_norm_mul(uint8_t a, uint8_t b) {
    return (uint8_t) (((uint32_t) (a) * b * 258u) >> 16u);
}

uint8_t u8_add_sat(uint8_t a, uint8_t b) {
    uint8_t c;
    return __builtin_add_overflow(a, b, &c) ? 0xFF : c;
}

#ifdef __cplusplus
}
#endif
