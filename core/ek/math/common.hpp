#pragma once

#include <cmath>

namespace ek::math {

constexpr double pi = 3.141592653589793238462643383279502884;
constexpr double pi2 = pi * 2.0;

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
inline T clamp(T x, T begin = T(0), T end = T(1)) {
    return x < begin ? begin : (x > end ? end : x);
}

template<typename T>
inline constexpr T to_radians(T degrees) noexcept {
    return degrees * pi / T(180);
}

template<typename T>
inline constexpr T to_degrees(T radians) noexcept {
    return radians * T(180) / pi;
}

template<typename T>
inline T saturate(T x) {
    return clamp(x);
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
    float t = 0.5f + 0.5f * sinf(pi2 * (frequency * time - 0.25f));
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

/* Thanks to good old Bit Twiddling Hacks for this one: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2 */
inline unsigned int nextPowerOf2(unsigned int x) {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;
    return x;
}

}