#pragma once

#include <ek/math/Math.hpp>

namespace ek {

struct Back {
    float overshoot = 1.70158f;

    [[nodiscard]]
    float calculate(float t) const {
        return t >= 1.0f ? 1.0f : t * t * ((overshoot + 1.0f) * t - overshoot);
    }
};

struct Bounce {
    static constexpr float B1 = 1.0f / 2.75f;
    static constexpr float B2 = 2.0f / 2.75f;
    static constexpr float B3 = 1.5f / 2.75f;
    static constexpr float B4 = 2.5f / 2.75f;
    static constexpr float B5 = 2.25f / 2.75f;
    static constexpr float B6 = 2.625f / 2.75f;

    [[nodiscard]]
    float calculate(float t) const {
        t = 1.0f - t;
        if (t < B1) return 1.0f - 7.5625f * t * t;
        if (t < B2) return 1.0f - (7.5625f * (t - B3) * (t - B3) + 0.75f);
        if (t < B4) return 1.0f - (7.5625f * (t - B5) * (t - B5) + 0.9375f);
        return 1.0f - (7.5625f * (t - B6) * (t - B6) + 0.984375f);
    }
};

struct Circ {

    [[nodiscard]]
    float calculate(float t) const {
        return 1.0f - sqrtf(1.0f - t * t);
    }
};

struct Elastic {
    float amplitude = 0.1f;
    float period = 0.4f;

    [[nodiscard]]
    float calculate(float t) const {
        if (t <= 0.0f) return 0.0f;
        if (t >= 1.0f) return 1.0f;
        float s;
        float a = amplitude;
        if (a < 1.0f) {
            a = 1.0f;
            s = period / 4.0f;
        } else {
            s = period / (Math::fPI2 * asinf(1.0f / a));
        }
        const float tm1 = t - 1.0f;
        return -(a * powf(2.0f, 10.0f * tm1) * sinf((tm1 - s) * Math::fPI2 / period));
    }
};

struct Expo {

    [[nodiscard]]
    float calculate(float t) const {
        return t <= 0.0f ? 0.0f : ::pow(2.0f, 10.0f * (t - 1.0f));
    }
};

struct Linear {

    [[nodiscard]]
    float calculate(float t) const {
        return t;
    }
};

struct Polynomial {
    float degree = 3.0f;

    [[nodiscard]]
    float calculate(float t) const {
        return ::pow(t, degree);
    }
};

struct Sine {
    [[nodiscard]]
    float calculate(float t) const {
        return t < 1.0f ? (1.0f - ::cos(t * Math::fPI_2)) : 1.0f;
    }
};

struct Step {
    [[nodiscard]]
    float calculate(float t) const {
        return t < 1.0f ? 0.0f : 1.0f;
    }
};

template<class T>
struct EaseInOut {
    T eq;

    [[nodiscard]]
    float calculate(float t) const {
        return t < 0.5f ? 0.5f * eq.calculate(t * 2.0f) : 1.0f - 0.5f * eq.calculate(2.0f - t * 2.0f);
    }
};

template<class T>
struct EaseOut {
    T eq;

    [[nodiscard]]
    float calculate(float t) const {
        return 1.0f - eq.calculate(1.0f - t);
    }
};


template<class T>
struct EaseOutIn {
    T eq;

    [[nodiscard]]
    float calculate(float t) const {
        return t < 0.5f ? 0.5f * (1.0f - eq.calculate(1.0f - t * 2.0f)) : 0.5f + 0.5f * eq.calculate(t * 2.0f - 1.0f);
    }
};

namespace easing {

static constexpr Polynomial P2_IN{2};
static constexpr EaseOut<Polynomial> P2_OUT{P2_IN};

static constexpr Polynomial P3_IN{3};
static constexpr EaseOut<Polynomial> P3_OUT{P3_IN};
static constexpr EaseInOut<Polynomial> P3_IN_OUT{P3_IN};

static constexpr Polynomial P4_IN{4};

static constexpr Elastic ELASTIC_IN;
static constexpr EaseOut<Elastic> ELASTIC_OUT{ELASTIC_IN};

static constexpr Circ CIRC_IN;
static constexpr EaseOut<Circ> CIRC_OUT{CIRC_IN};
static constexpr EaseInOut<Circ> CIRC_IN_OUT{CIRC_IN};

static constexpr Bounce BOUNCE_IN;
static constexpr EaseOut<Bounce> BOUNCE_OUT{BOUNCE_IN};

static constexpr Back BACK_IN;
static constexpr EaseOut<Back> BACK_OUT{BACK_IN};

}
}
