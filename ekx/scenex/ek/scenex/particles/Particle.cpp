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
            color.a = alpha * a;
        }
            break;
        case particle_alpha_mode::QuadOut: {
            float x = 1.0f - time / time_total;
            color.af(alpha * (1.0f - x * x));
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
    }
    return bounds;
}

void Particle::draw() {
    float vis_angle = angle_base + rotation + angle_velocity_factor * atan2f(velocity.y, velocity.x);

    draw2d::state.save_transform()
            .transform_pivot(position, vis_angle, scale, pivot)
            .concat(color, offset);
    if (sprite) {
        sprite->draw();
    }
    draw2d::state.restore_transform();
}

}