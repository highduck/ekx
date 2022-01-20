#pragma once

#include "ParticleDecl.hpp"

#include <ek/util/Res.hpp>
#include <ek/scenex/2d/Sprite.hpp>
#include <ek/scenex/text/Font.hpp>
#include <ek/canvas.h>

namespace ek {

struct Particle {
    REF_TO(Sprite) sprite = 0;
    //int draw_layer = 0;
    //const char* text = nullptr;
    vec2_t pivot = {};

    float time = 0.0f;
    float time_total = 0.0f;
    vec2_t position = {};
    vec2_t velocity = {};
    vec2_t acc = {};

    float acc_x_phase{MATH_PI / 2.0f};
    float acc_x_speed = 0.0f;

    // angle state
    float angle_base = 0.0f;

    // rotating and rotation speed
    float rotation = 0.0f;
    float rotation_speed = 0.0f;
    float angle_velocity_factor = 0.0f;

    // initial alpha
    ParticleAlphaMode alpha_mode = ParticleAlphaMode::None;
    float alpha = 1.0f;
    color_t color = COLOR_WHITE;
    color_t offset = COLOR_ZERO;

    ParticleScaleMode scale_mode = ParticleScaleMode::None;
    float scale_off_time = 0.0f;
    float scale_start = 1.0f;
    float scale_end = 0.0f;

    // current state
    vec2_t scale = vec2(1,1);

    bool reflector = false;

    rect_t bounds = rect_01();
public:

    void init() {
        update_current_values();
    }

    [[nodiscard]] inline bool is_alive() const {
        return time > 0.0f;
    }

    inline void set_life_time(float life_time) {
        time = time_total = life_time;
    }

    void update(float dt);

    void update_current_values();

    rect_t get_bounds();

    void draw();
};

}


