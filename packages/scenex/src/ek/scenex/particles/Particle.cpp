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
        case ParticleScaleMode::CosOut: {
            float s = cosf(0.5f * float(MATH_PI) * (1.0f - time / time_total));
            scale = vec2(s, s);
        }
            break;
        case ParticleScaleMode::Range: {
            float time_max = scale_off_time > 0.0f ? scale_off_time : time_total;
            float ratio = 1.0f - saturate(time / time_max);
            float s = lerp_f32(scale_start, scale_end, ratio);
            scale = vec2(s, s);
        }
            break;
        default:
            scale = vec2(scale_start, scale_start);
            break;
    }

    switch (alpha_mode) {
        case ParticleAlphaMode::ByScale:
            color.a = unorm8_f32_clamped(alpha * length_vec2(scale));
            break;
        case ParticleAlphaMode::LifeSin:
            color.a = unorm8_f32_clamped(alpha * sinf(MATH_PI * time / time_total));
            break;
        case ParticleAlphaMode::DCBlink: {
            float a = 0.25f;
            if (time > 0.75f) {
                a = 1.0f - time;
            } else if (time < 0.25f) {
                a = time;
            }
            a *= 4.0f;
            color.a = unorm8_f32_clamped(alpha * a);
        }
            break;
        case ParticleAlphaMode::QuadOut: {
            float x = 1.0f - time / time_total;
            color.a = unorm8_f32_clamped(alpha * (1.0f - x * x));
        }
            break;
        default:
            color.a = unorm8_f32_clamped(alpha);
            break;
    }
}

rect_t Particle::get_bounds() {
    if (sprite) {
        bounds = sprite->rect;
    }
    return bounds;
}

void Particle::draw() {
    float vis_angle = angle_base + rotation + angle_velocity_factor * atan2f(velocity.y, velocity.x);

    canvas_save_transform();
    canvas_transform_pivot(position, vis_angle, scale, pivot);
    canvas_concat_color({color, offset});
    if (sprite) {
        sprite->draw();
    }
    canvas_restore_transform();
}

}