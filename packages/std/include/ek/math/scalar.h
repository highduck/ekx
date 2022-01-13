#ifndef EK_MATH_SCALAR_H
#define EK_MATH_SCALAR_H

#ifdef __cplusplus
extern "C" {
#endif

#define MATH_PI (3.141592653589793238462643383279502884f)
#define MATH_TAU ( MATH_PI * 2 )
#define MATH_F32_EPSILON (1.0e-6f)
#define MATH_F64_EPSILON (1.0e-8)

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

/**
 * Constructs `f32` value in range [0; 1) from full `u32` bits representation
 * @param x - any `u32` value
 * @return unorm `f32` value in range [0; 1)
 */
float unorm_f32_from_u32(uint32_t x);

float to_radians(float degrees);
float to_degrees(float radians);

inline static float square(float x) {
    return x * x;
}

inline static int sign(float v) {
    return v > 0.0f ? 1 : (v < 0.0f ? -1 : 0);
}

inline static float reach(float current, float target, float step) {
    if (current < target) {
        current += fabsf(step);
        if (current > target) {
            current = target;
        }
    } else if (current > target) {
        current -= fabsf(step);
        if (current < target) {
            current = target;
        }
    }
    return current;
}

inline static void reach_value(float* value, float target, float step) {
    *value = reach(*value, target, step);
}

inline static float reach_delta(float current, float target, float delta_up, float delta_down) {
    if (current < target && delta_up > 0) {
        current += delta_up;
        if (current > target) {
            current = target;
        }
    } else if (current > target && delta_down < 0) {
        current += delta_down;
        if (current < target) {
            current = target;
        }
    }
    return current;
}

inline static float reduce(float x, float period, float offset) {
    period = fabsf(period);
    return x - period * floorf((x - offset) / period) - offset;
}

inline static float fract(float x) {
    return x - floorf(x);
}

inline static float fract_positive_fast(float x) {
    return x - (float)((int)x);
}

inline static float osc_sine(float time, float min, float max, float frequency) {
    float t = 0.5f + 0.5f * sinf(MATH_TAU * (frequency * time - 0.25f));
    return min + (max - min) * t;
}

inline static float osc_circle(float time, float min, float max, float frequency) {
    float x = 2.0f * reduce(time * frequency, 1.0f, 0.0f) - 1.0f;
    float t = sqrtf(1.0f - x * x);
    return min + (max - min) * t;
}

inline static float integrate_exp(float k, float dt, float fps) {
    float c = logf(1.0f - k) * (fps > 0 ? fps : 60.0f);
    return 1.0f - expf(c * dt);
}


// returns next power of 2
uint32_t round_up_pow2(uint32_t x);

float lerp_f32(float a, float b, float t);

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

/// sin/cos utility

// normalized parameter sin/cos: (0 ... 1) => (-1 ~ 1 ~ -1)
// norm_sin(0.00) = 0
// norm_sin(0.25) = 1
// norm_sin(0.50) = 0
// norm_sin(0.75) = -1
inline static float norm_sin(float x){
    return sinf(MATH_TAU * x);
}

// norm_cos(0.00) = 1
// norm_cos(0.25) = 0
// norm_cos(0.50) = -1
// norm_cos(0.75) = 0
inline static float norm_cos(float x){
    return cosf(MATH_TAU * x);
}

// unit sin/cos: (0 ... 1) => (0 ~ 1 ~ 0)
// unit_sin(0.00) = 0.5
// unit_sin(0.25) = 1
// ...
inline static float unorm_sin(float x){
    return 0.5f + 0.5f * norm_sin(x);
}

inline static  float unorm_cos(float x){
    return 0.5f + 0.5f * norm_cos(x);
}

#ifdef __cplusplus
}
#endif

#endif // EK_MATH_SCALAR_H
