#pragma once

#include <scenex/config.h>
#include <ek/assets.hpp>
#include <scenex/2d/sprite.hpp>
#include <scenex/2d/font.hpp>
#include <ek/math/rand.hpp>

#include <vector>
#include <functional>

namespace scenex {

class particle;

enum class particle_scale_mode {
    None = 0,
    CosOut = 1,
    Range = 2
};

enum class particle_alpha_mode {
    None = 0,
    ByScale = 1,
    LifeSin = 2,
    DCBlink = 3
};

enum class rand_color_mode {
    Continuous = 0,
    RandLerp = 1,
    RandElement = 2
};

class rand_color_data {
public:
    std::vector<argb32_t> colors = {argb32_t{0xFFFFFFFF}};
    rand_color_mode mode = rand_color_mode::RandElement;
    mutable int state = 0;

    argb32_t next() const {
        ++state;
        int index_max = int(colors.size() - 1);
        if (colors.empty()) {
            return argb32_t{0xFFFFFFFF};
        }
        if (colors.size() == 1) return colors[0];
        switch (mode) {
            case rand_color_mode::RandLerp: {
                float t = ek::rand_fx.random(0.0f, float(index_max));
                int i = int(t);
                t = ek::math::fract(t);
                return lerp(colors[i], colors[i + 1], t);
            }
            case rand_color_mode::RandElement:
                return colors[ek::rand_fx.random_int(0, index_max)];
            case rand_color_mode::Continuous:
                return colors[state % colors.size()];
        }
        return argb32_t{0xFFFFFFFF};
    }

    void set_gradient(argb32_t color1, argb32_t color2) {
        colors = {color1, color2};
        mode = rand_color_mode::RandLerp;
    }

    void set_steps(argb32_t color1, argb32_t color2) {
        colors = {color1, color2};
        mode = rand_color_mode::Continuous;
    }

    void set_solid(argb32_t color) {
        colors = {color};
        mode = rand_color_mode::Continuous;
    }
};

class float_range_t {
public:
    float min;
    float max;

    float_range_t()
            : min{0.0f},
              max{0.0f} {
    }

    explicit float_range_t(float value)
            : min{value},
              max{value} {
    }

    explicit float_range_t(float min_, float max_)
            : min{min_},
              max{max_} {
    }

    float_range_t& set(float a_min, float a_max) {
        min = a_min;
        max = a_max;
        return *this;
    }

    float_range_t& set(float value) {
        min = value;
        max = value;
        return *this;
    }

    [[nodiscard]] inline float random() const {
        return ek::rand_fx.random(min, max);
    }
};

class particle_decl {
public:
    ek::asset_t<sprite_t> sprite;
    ek::asset_t<font_t> font;
    int font_size = 0;
    particle_scale_mode scale_mode = particle_scale_mode::None;
    particle_alpha_mode alpha_mode = particle_alpha_mode::None;
    float_range_t alpha_start{1.0};
    float2 acceleration = float2::zero;
    bool use_reflector = false;

    float_range_t life_time{1.0f};
    float_range_t acc_x_phase{0.5f * ek::math::pi};
    float_range_t acc_x_speed{0.0f};

    float scale_off_time = 0.0f;
    float_range_t scale_start{1.0f};
    float_range_t scale_end{0.0f};

    rand_color_data color;
    argb32_t color_offset{0x0};
    float additive = 0.0f;

    float_range_t rotation{0.0f};
    float rotation_speed{0.0f};

    float angle_velocity_factor = 0.0f;
    float angle_base = 0.0f;

    std::function<argb32_t(particle&)> color_func;

};

class emitter_data {
public:
    float interval = 0.5f;
    int burst = 1;
    rect_f rect;
    float2 offset;

    float_range_t burst_rotation_delta{1.0f, 1.5f};

    float_range_t speed{10, 100};
    float_range_t acc{0.0f};
    float_range_t dir{0.0f, ek::math::pi2};
};

}


