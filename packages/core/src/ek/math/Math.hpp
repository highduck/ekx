#pragma once

#include <cmath>
#include <ek/math.h>

namespace ek::Math {

template<typename T>
T epsilon();

template<>
inline float epsilon() { return 1e-6f; }

template<>
inline double epsilon() { return 1e-8; }

template<typename T>
inline T lerp(T begin, T end, T t) {
    return begin + (end - begin) * t;
}

template<typename T>
inline bool equals(T a, T b) {
    return std::abs(a - b) < epsilon<T>();
    // carefully with these bugs
    // return a < b + epsilon<T>() && a > b - epsilon<T>();
}

template<typename T>
T reach(T current, T target, T step) {
    if (current < target) {
        current += std::abs(step);
        if (current > target) {
            current = target;
        }
    } else if (current > target) {
        current -= std::abs(step);
        if (current < target) {
            current = target;
        }
    }
    return current;
}

inline void reachValue(float& value, float target, float step) {
    value = reach(value, target, step);
}

template<typename T>
T reach_delta(T current, T target, T delta_up, T delta_down) {
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

template<typename T>
T reduce(T x, T period, T offset) {
    period = std::abs(period);
    return x - period * floor((x - offset) / period) - offset;
}

template<typename T>
T fract(T x) {
    return x - floor(x);
}

template<typename T>
T fract_positive_fast(T x) {
    return x - static_cast<int>(x);
}

inline float osc_sine(float time, float min, float max, float frequency) {
    float t = 0.5f + 0.5f * sinf(2 * MATH_PI * (frequency * time - 0.25f));
    return min + (max - min) * t;
}

inline float osc_circle(float time, float min, float max, float frequency) {
    float x = 2.0f * reduce(time * frequency, 1.0f, 0.0f) - 1.0f;
    float t = sqrtf(1.0f - x * x);
    return min + (max - min) * t;
}

inline int sign(float v) {
    return v > 0.0f ? 1 : (v < 0.0f ? -1 : 0);
}

inline float Q_rsqrt(float x) {
//    const float x2 = number * 0.5f;
//    const float threehalfs = 1.5f;
//
//    union {
//        float f;
//        uint32_t i;
//    } conv = {.f = number};
//    conv.i = 0x5f3759df - (conv.i >> 1);
//    conv.f *= threehalfs - (x2 * conv.f * conv.f);
//    return conv.f;
    return 1.0f / sqrtf(x);
}

//
//union FloatInt {
//    int32_t i;
//    float f;
//};
//
//inline int32_t fastFloatToInt(float x) {
//    constexpr FloatInt magic{(150 << 23) | (1 << 22)};
//    return FloatInt{
//            .f = x + magic.f
//    }.i - magic.i;
//}

inline float integrateExp(float k, float dt, float fps = 60.0f) {
    float c = logf(1.0f - k) * fps;
    return 1.0f - expf(c * dt);
}

// normalized parameter sin/cos: (0 ... 1) => (-1 ~ 1 ~ -1)
// normSin(0.00) = 0
// normSin(0.25) = 1
// normSin(0.50) = 0
// normSin(0.75) = -1
inline float normSin(float x){
    return sinf(2 * MATH_PI * x);
}

// normCos(0.00) = 1
// normCos(0.25) = 0
// normCos(0.50) = -1
// normCos(0.75) = 0
inline float normCos(float x){
    return cosf(2 * MATH_PI * x);
}

// unit sin/cos: (0 ... 1) => (0 ~ 1 ~ 0)
// unitSin(0.00) = 0.5
// unitSin(0.25) = 1
// ...
inline float unitSin(float x){
    return 0.5f + 0.5f * normSin(x);
}

inline float unitCos(float x){
    return 0.5f + 0.5f * normCos(x);
}

}
