#pragma once

#include "ParticleDecl.hpp"

#include <ek/util/Res.hpp>
#include <ek/scenex/2d/Sprite.hpp>
#include <ek/scenex/text/Font.hpp>
#include <ek/draw2d/drawer.hpp>

namespace ek {

struct Particle {
    Res<Sprite> sprite;
    //int draw_layer = 0;
    //const char* text = nullptr;
    Vec2f pivot = Vec2f::zero;

    float time = 0.0f;
    float time_total = 0.0f;
    Vec2f position = Vec2f::zero;
    Vec2f velocity = Vec2f::zero;
    Vec2f acc = Vec2f::zero;

    float acc_x_phase{Math::pi / 2.0f};
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
    argb32_t color{0xFFFFFFFF};
    argb32_t offset{0x0};

    ParticleScaleMode scale_mode = ParticleScaleMode::None;
    float scale_off_time = 0.0f;
    float scale_start = 1.0f;
    float scale_end = 0.0f;

    // current state
    Vec2f scale = Vec2f::one;

    bool reflector = false;

    Rect2f bounds = Rect2f::zero_one;
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

    Rect2f get_bounds();

    void draw();
};

}


