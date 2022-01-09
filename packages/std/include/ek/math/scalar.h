#ifndef EK_MATH_SCALAR_H
#define EK_MATH_SCALAR_H

#ifdef __cplusplus
extern "C" {
#endif

#define MATH_PI (3.141592653589793238462643383279502884f)
#define MATH_F32_EPSILON (1.0e-6f)
#define MATH_F64_EPSILON (1.0e-8)

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

float to_radians(float degrees);
float to_degrees(float radians);

uint32_t next_power_of_2(uint32_t x);

float f32_lerp(float a, float b, float t);

bool almost_eq_f32(float a, float b, float eps);

inline static float clamp(float x, float min, float max) {
    return x > max ? max : (x < min ? min : x);
}

inline static float saturate(float x) {
    return clamp(x, 0, 1);
}

/** u8 functions **/

uint8_t unorm8_f32(float f);
uint8_t unorm8_f32_clamped(float f);
uint8_t u8_norm_mul(uint8_t a, uint8_t b);
uint8_t u8_add_sat(uint8_t a, uint8_t b);

#ifdef __cplusplus
}
#endif

#endif // EK_MATH_SCALAR_H
