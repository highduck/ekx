#pragma once

#include "particle_decl.h"

#include <scenex/config.h>
#include <ek/assets.hpp>
#include <scenex/2d/sprite.hpp>
#include <scenex/2d/font.hpp>
#include <draw2d/drawer.hpp>

namespace scenex {

class particle {
public:
    ek::asset_t<sprite_t> sprite;
    int draw_layer = 0;
    std::string text;
    ek::asset_t<font_t> font;
    int font_size = 0;
    float2 pivot = float2::zero;

    float time = 0.0f;
    float time_total = 0.0f;
    float2 position = float2::zero;
    float2 velocity = float2::zero;
    float2 acc = float2::zero;

    float acc_x_phase{ek::math::pi / 2.0f};
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

    std::function<argb32_t(particle & )> color_functor;

    void init() {
        update_current_values();
    }

    inline bool is_alive() {
        return time > 0.0f;
    }

    inline void set_life_time(float life_time) {
        time = time_total = life_time;
    }

    void update(float dt) {
        float ax = acc.x * sinf(acc_x_phase + time * acc_x_speed);
        velocity.x += ax * dt;
        velocity.y += acc.y * dt;

        position.x += velocity.x * dt;
        position.y += velocity.y * dt;

        if (reflector && position.y + 4.0f > 0.0f) {
            position.y = -4.0f;
            velocity.y = -0.5f * velocity.y;
            velocity.x = 0.7f * velocity.x;
        }

        rotation += rotation_speed * dt;

        update_current_values();

        time -= dt;
    }

    void update_current_values() {
        switch (scale_mode) {
            case particle_scale_mode::CosOut: {
                float s = cosf(0.5f * float(ek::math::pi) * (1.0f - time / time_total));
                scale = float2(s, s);
            }
                break;
            case particle_scale_mode::Range: {
                float time_max = scale_off_time > 0.0f ? scale_off_time : time_total;
                float ratio = 1.0f - ek::math::clamp(time / time_max);
                float s = ek::math::lerp(scale_start, scale_end, ratio);
                scale = float2(s, s);
            }
                break;
            default:
                scale = float2(scale_start, scale_start);
                break;
        }

        if (color_functor) {
            color = color_functor(*this);
        }

        switch (alpha_mode) {
            case particle_alpha_mode::ByScale:
                color.af(ek::math::clamp(alpha * length(scale)));
                break;
            case particle_alpha_mode::LifeSin:
                color.af(alpha * sinf(float(ek::math::pi) * time / time_total));
                break;
            case particle_alpha_mode::DCBlink: {
                float a = 0.25f;
                if (time > 0.75f) {
                    a = 1.0f - time;
                } else if (time < 0.25f) {
                    a = time;
                }
                a *= 4.0f;
                color.af(alpha * a);
            }
                break;
            default:
                color.af(alpha);
                break;
        }
    }

    rect_f get_bounds() {
        if (sprite) {
            bounds = sprite->rect;
        } else if (font_size > 0 && font && !text.empty()) {
            float width = font->get_text_segment_width(text, font_size, 0, int(text.size()));
            bounds.set(-0.5f * width, -0.5f * float(font_size), width, font_size);
        }
        return bounds;
    }

    void draw_cycled(ek::drawer_t& drawer) {
        auto camera = drawer.canvas_rect();
        float width = camera.width;
        auto box = translate(get_bounds().scale(scale), position);
        if (box.right() >= camera.x && box.x <= camera.right()) {
            draw(drawer, 0);
        }
        if (box.right() > camera.right() && box.right() - width >= camera.x && box.x - width <= camera.right()) {
            draw(drawer, -width);
        }
        if (bounds.x < camera.x && box.right() + width >= camera.x && box.x + width <= camera.right()) {
            draw(drawer, width);
        }
    }

    void draw(ek::drawer_t& drawer, float offset_x) {
        float vis_angle = angle_base + rotation + angle_velocity_factor * atan2f(velocity.y, velocity.x);
        float2 pos = position + float2(offset_x, 0.0f);

        drawer.save_transform();
        drawer.transform_pivot(pos, vis_angle, scale, pivot);
        drawer.combine_color(color, offset);
        {
            const auto size = static_cast<float>(font_size);
            if (sprite) {
                sprite->draw();
            } else if (size > 0 && font && !text.empty()) {
                float width = font->get_text_segment_width(text, size, 0, static_cast<int>(text.size()));
                font->draw(text, size, float2(-0.5f * width, 0.5f * size), argb32_t{0xFFFFFFFF},
                           size);
            }
        }
        drawer.restore_transform();
    }
};

}


