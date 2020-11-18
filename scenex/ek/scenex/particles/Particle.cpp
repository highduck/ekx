#include "Particle.hpp"

namespace ek {

void Particle::update(float dt) {
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

void Particle::update_current_values() {
    switch (scale_mode) {
        case particle_scale_mode::CosOut: {
            float s = cosf(0.5f * float(math::pi) * (1.0f - time / time_total));
            scale = float2(s, s);
        }
            break;
        case particle_scale_mode::Range: {
            float time_max = scale_off_time > 0.0f ? scale_off_time : time_total;
            float ratio = 1.0f - math::clamp(time / time_max);
            float s = math::lerp(scale_start, scale_end, ratio);
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
            color.af(math::clamp(alpha * length(scale)));
            break;
        case particle_alpha_mode::LifeSin:
            color.af(alpha * sinf(float(math::pi) * time / time_total));
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

rect_f Particle::get_bounds() {
    if (sprite) {
        bounds = sprite->rect;
    } else if (font_size > 0 && font && !text.empty()) {
        float width = font->get_text_segment_width(text, font_size, 0, int(text.size()));
        bounds.set(-0.5f * width, -0.5f * float(font_size), width, font_size);
    }
    return bounds;
}

void Particle::draw_cycled() {
    auto camera = draw2d::state.canvas_rect;
    float width = camera.width;
    auto box = translate(get_bounds().scale(scale), position);
    if (box.right() >= camera.x && box.x <= camera.right()) {
        draw(0);
    }
    if (box.right() > camera.right() && box.right() - width >= camera.x && box.x - width <= camera.right()) {
        draw(-width);
    }
    if (bounds.x < camera.x && box.right() + width >= camera.x && box.x + width <= camera.right()) {
        draw(width);
    }
}

void Particle::draw(float offset_x) {
    float vis_angle = angle_base + rotation + angle_velocity_factor * atan2f(velocity.y, velocity.x);
    float2 pos = position + float2(offset_x, 0.0f);

    draw2d::state
            .save_transform()
            .transform_pivot(pos, vis_angle, scale, pivot)
            .concat(color, offset);
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
    draw2d::state.restore_transform();
}

}