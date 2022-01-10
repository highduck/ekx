#pragma once

#include <ek/util/Res.hpp>
#include <ek/scenex/2d/Sprite.hpp>
#include <ek/scenex/text/Font.hpp>
#include <ek/math/Random.hpp>

namespace ek {

enum class ParticleScaleMode {
    None = 0,
    CosOut = 1,
    Range = 2
};

enum class ParticleAlphaMode {
    None = 0,
    ByScale = 1,
    LifeSin = 2,
    DCBlink = 3,
    QuadOut = 4
};

enum class RandColorMode {
    Continuous = 0,
    RandLerp = 1,
    RandElement = 2
};

class RandColorData {
public:
    Array <color_t> colors{COLOR_WHITE};
    RandColorMode mode = RandColorMode::RandElement;
    mutable int state = 0;

    color_t next() const {
        ++state;
        int index_max = int(colors.size() - 1);
        if (colors.empty()) {
            return COLOR_WHITE;
        }
        if (colors.size() == 1) return colors[0];
        switch (mode) {
            case RandColorMode::RandLerp: {
                float t = rand_fx.random(0.0f, float(index_max));
                int i = int(t);
                t = fract(t);
                return lerp_color(colors[i], colors[i + 1], t);
            }
            case RandColorMode::RandElement:
                return colors[rand_fx.random_int(0, index_max)];
            case RandColorMode::Continuous:
                return colors[state % colors.size()];
        }
        return COLOR_WHITE;
    }

    void set_gradient(color_t color1, color_t color2) {
        colors = {color1, color2};
        mode = RandColorMode::RandLerp;
    }

    void set_steps(color_t color1, color_t color2) {
        colors = {color1, color2};
        mode = RandColorMode::Continuous;
    }

    void set_solid(color_t color) {
        colors = {color};
        mode = RandColorMode::Continuous;
    }
};

struct FloatRange {
    float min;
    float max;

    FloatRange() :
            min{0.0f},
            max{0.0f} {
    }

    explicit FloatRange(float value) :
            min{value},
            max{value} {
    }

    explicit FloatRange(float min_, float max_) :
            min{min_},
            max{max_} {
    }

    FloatRange& set(float a_min, float a_max) {
        min = a_min;
        max = a_max;
        return *this;
    }

    FloatRange& set(float value) {
        min = value;
        max = value;
        return *this;
    }

    [[nodiscard]] inline float random() const {
        return rand_fx.random(min, max);
    }

    [[nodiscard]] const float* data() const {
        return reinterpret_cast<const float*>(this);
    }

    [[nodiscard]] float* data() {
        return reinterpret_cast<float*>(this);
    }
};

struct ParticleDecl {
    Res <Sprite> sprite;
    ParticleScaleMode scale_mode = ParticleScaleMode::None;
    ParticleAlphaMode alpha_mode = ParticleAlphaMode::None;
    FloatRange alpha_start{1.0};
    vec2_t acceleration = {};
    bool use_reflector = false;

    FloatRange life_time{1.0f};
    FloatRange acc_x_phase{0.5f * MATH_PI};
    FloatRange acc_x_speed{0.0f};

    float scale_off_time = 0.0f;
    FloatRange scale_start{1.0f};
    FloatRange scale_end{0.0f};

    RandColorData color;
    color_t color_offset = COLOR_ZERO;
    float additive = 0.0f;

    FloatRange rotation{0.0f};
    float rotation_speed{0.0f};

    float angle_velocity_factor = 0.0f;
    float angle_base = 0.0f;
};

struct EmitterData {
    float interval = 0.5f;
    int burst = 1;
    rect_t rect = {};
    vec2_t offset = {};

    FloatRange burst_rotation_delta{1.0f, 1.5f};

    FloatRange speed{10, 100};
    FloatRange acc{0.0f};
    FloatRange dir{0.0f, MATH_TAU};
};

EK_DECLARE_TYPE(ParticleDecl);
EK_TYPE_INDEX(ParticleDecl, 10);

}


