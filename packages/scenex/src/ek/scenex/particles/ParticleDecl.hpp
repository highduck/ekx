#pragma once

#include <ek/scenex/2d/Sprite.hpp>
#include <ek/rnd.h>
#include <ek/rr.h>
#include <ek/ds/Array.hpp>

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
    Array <color_t> colors;
    RandColorMode mode = RandColorMode::RandElement;
    mutable uint32_t state;

    RandColorData() {
        colors.push_back(COLOR_WHITE);
        mode = RandColorMode::RandElement;
        state = 0;
    }

    color_t next() const {
        ++state;
        uint32_t size = colors.size();
        if (!size) {
            return COLOR_WHITE;
        }
        if (size == 1) {
            return colors[0];
        }
        switch (mode) {
            case RandColorMode::RandLerp: {
                float t = random_f();
                uint32_t i = random_n(size - 1);
                return lerp_color(colors[i], colors[i + 1], t);
            }
            case RandColorMode::RandElement:
                return colors[random_n(colors.size())];
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
        return random_range_f(min, max);
    }

    [[nodiscard]] const float* data() const {
        return reinterpret_cast<const float*>(this);
    }

    [[nodiscard]] float* data() {
        return reinterpret_cast<float*>(this);
    }
};

struct ParticleDecl {
    R(sprite_t) sprite;
    ParticleScaleMode scale_mode;
    ParticleAlphaMode alpha_mode;
    FloatRange alpha_start;
    vec2_t acceleration;
    bool use_reflector;

    FloatRange life_time;
    FloatRange acc_x_phase;
    FloatRange acc_x_speed;

    float scale_off_time;
    FloatRange scale_start;
    FloatRange scale_end;

    RandColorData color;
    color_t color_offset;
    float additive;

    FloatRange rotation;
    float rotation_speed;
    float angle_velocity_factor;
    float angle_base;
};

struct res_particle {
    string_hash_t names[32];
    ParticleDecl data[32];
    rr_man_t rr;
};

#define R_PARTICLE(name) REF_NAME(::ek::res_particle, name)

inline struct res_particle res_particle;

inline void setup_res_particle(void) {
    struct res_particle* R = &res_particle;
    rr_man_t* rr = &R->rr;

    rr->names = R->names;
    rr->data = R->data;
    rr->max = sizeof(R->data) / sizeof(R->data[0]);
    rr->num = 1;
    rr->data_size = sizeof(R->data[0]);
}

inline void particle_decl_init(ParticleDecl* decl) {
//    memset(decl, 0, sizeof(ParticleDecl));
    decl->alpha_start = FloatRange{1.0};
    decl->life_time = FloatRange{1.0f};
    decl->acc_x_phase = FloatRange{0.5f * MATH_PI};
    decl->acc_x_speed = FloatRange{0.0f};
    decl->scale_start = FloatRange{1.0f};
    decl->scale_end = FloatRange{0.0f};
//    decl->color = RandColorData();
    decl->rotation = FloatRange{0.0f};
}

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

}


