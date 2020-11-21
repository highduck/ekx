#pragma once

#include "ParticleDecl.hpp"

#include <ek/util/assets.hpp>
#include <ek/scenex/2d/Sprite.hpp>
#include <ek/scenex/text/Font.hpp>
#include <ek/draw2d/drawer.hpp>

namespace ek {

struct Particle {
    Res<Sprite> sprite;
    int draw_layer = 0;
    std::string text;
    Res<Font> font;
    float fontSize = 0.0f;
    float2 pivot = float2::zero;

    float time = 0.0f;
    float time_total = 0.0f;
    float2 position = float2::zero;
    float2 velocity = float2::zero;
    float2 acc = float2::zero;

    float acc_x_phase{math::pi / 2.0f};
    float acc_x_speed = 0.0f;

    // angle state
    float angle_base = 0.0f;

    // rotating and rotation speed
    float rotation = 0.0f;
    float rotation_speed = 0.0f;
    float angle_velocity_factor = 0.0f;

    // initial alpha
    particle_alpha_mode alpha_mode = particle_alpha_mode::None;
    float alpha = 1.0f;
    argb32_t color{0xFFFFFFFF};
    argb32_t offset{0x0};

    particle_scale_mode scale_mode = particle_scale_mode::None;
    float scale_off_time = 0.0f;
    float scale_start = 1.0f;
    float scale_end = 0.0f;

    // current state
    float2 scale = float2::one;

    bool reflector = false;

    rect_f bounds = rect_f::zero_one;

    std::function<argb32_t(Particle&)> color_functor;

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

    rect_f get_bounds();

    void draw();
};

}


