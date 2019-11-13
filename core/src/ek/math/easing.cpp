#include "easing.hpp"
#include <cmath>

namespace ek {

basic_ease_t::~basic_ease_t() = default;

float Back::calculate(float t) const {
    return t >= 1.0f ? 1.0f : t * t * ((overshoot + 1.0f) * t - overshoot);
}

float Bounce::calculate(float t) const {
    t = 1.0f - t;
    if (t < B1) return 1.0f - 7.5625f * t * t;
    if (t < B2) return 1.0f - (7.5625f * (t - B3) * (t - B3) + 0.75f);
    if (t < B4) return 1.0f - (7.5625f * (t - B5) * (t - B5) + 0.9375f);
    return 1.0f - (7.5625f * (t - B6) * (t - B6) + 0.984375f);
}

float Circ::calculate(float t) const {
    return 1.0f - sqrtf(1.0f - t * t);
}

float Elastic::calculate(float t) const {
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    float s;
    float a = amplitude;
    if (a < 1.0f) {
        a = 1.0f;
        s = period / 4.0f;
    } else {
        s = period / (2.0f * M_PI) * asinf(1.0f / a);
    }
    const float tm1 = t - 1.0f;
    return -(a * powf(2.0f, 10.0f * tm1) * sinf((tm1 - s) * (2.0f * M_PI) / period));
}


float Expo::calculate(float t) const {
    return t <= 0.0f ? 0.0f : powf(2.0f, 10.0f * (t - 1.0f));
}

float Linear::calculate(float t) const {
    return t;
}

float Polynomial::calculate(float t) const {
    return powf(t, degree);
}

float Sine::calculate(float t) const {
    return t >= 1.0f ? 1.0f : 1.0f - cosf(t * M_PI * 0.5f);
}

float Step::calculate(float t) const {
    return t < 1.0f ? 0.0f : 1.0f;
}

}
