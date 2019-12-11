#include "movie_clip_system.h"

#include "game_time.h"
#include <ek/scenex/components/transform_2d.h>

namespace ek {

using ecs::entity;

float ease(float x, const easing_data_t& data);

void apply_frame(entity e, movie_t& mov);

void update_movie_clips() {
    for (auto e : ecs::view<movie_t>()) {
        auto& mov = ecs::get<movie_t>(e);
        auto dt = get_delta_time(e);
        if (mov.playing) {
            mov.time += dt * mov.fps;
            mov.trunc_time();
            apply_frame(e, mov);
        }
    }
}

void apply_frame(entity e, movie_t& mov) {
    auto* data = mov.get_movie_data();
    auto time = mov.time;
    if (!data) {
        // no data - exit early
        return;
    }
    for (const auto& layer : data->layers) {
        int keyframe_index = 0;
        int animation_key = 0;
        const int keyframes_count = layer.frames.size();
        for (; keyframe_index < keyframes_count; ++keyframe_index) {
            const auto& k1 = layer.frames[keyframe_index];
            if (time >= k1.index && time < k1.index + k1.duration) {
                animation_key = k1.key;
                break;
            }
        }

        ecs::entity target{};
        ecs::entity child = ecs::get<node_t>(e).child_first;
        while (child) {
            auto& movie_keys = ecs::get<movie_target_keys>(child);
            if (movie_keys.key_layer == layer.key) {
                auto& config = ecs::get_or_create<node_state_t>(child);
                if (animation_key == movie_keys.key_animation) {
                    target = child;
                    config.visible = true;
                } else {
                    config.visible = false;
                }
            }
            child = ecs::get<node_t>(child).sibling_next;
        }
        if (target) {
            const auto& k1 = layer.frames[keyframe_index];
            const auto& k2 = (keyframe_index + 1) < layer.frames.size() ? layer.frames[keyframe_index + 1]
                                                                        : layer.frames[keyframe_index];
            auto& transform = ecs::get<transform_2d>(target);
            float2 position;
            if (k1.motion_type == 1) {
                const float progress = (time - static_cast<float>(k1.index)) / static_cast<float>(k1.duration);
                float x_position = progress;
                float x_rotation = progress;
                float x_scale = progress;
                float x_color = progress;
                for (const auto& easing_data : k1.tweens) {
                    const float x = ease(progress, easing_data);
                    if (easing_data.attribute == 0) {
                        x_position = x_rotation = x_scale = x_color = x;
                        break;
                    } else if (easing_data.attribute == 1) {
                        x_position = x;
                    } else if (easing_data.attribute == 2) {
                        x_rotation = x;
                    } else if (easing_data.attribute == 3) {
                        x_scale = x;
                    } else if (easing_data.attribute == 4) {
                        x_color = x;
                    }
                }

                position = lerp(k1.position, k2.position, x_position);
                transform.skew = lerp(k1.skew, k2.skew, x_rotation);
                transform.scale = lerp(k1.scale, k2.scale, x_scale);
                transform.colorMultiplier = argb32_t{
                        lerp(k1.color.multiplier, k2.color.multiplier, x_color)
                };
                transform.colorOffset = argb32_t{
                        lerp(k1.color.offset, k2.color.offset, x_color)
                };
            } else {
                position = k1.position;
                transform.skew = k1.skew;
                transform.scale = k1.scale;
                transform.colorMultiplier = argb32_t{k1.color.multiplier};
                transform.colorOffset = argb32_t{k1.color.offset};
            }
            auto& m = transform.matrix;
            m.set(transform.scale, transform.skew);
            m.tx = position.x - m.a * k1.pivot.x - m.c * k1.pivot.y;
            m.ty = position.y - m.b * k1.pivot.x - m.d * k1.pivot.y;
            transform.matrix = m;
        }
    }
}

void goto_and_stop(entity e, int frame) {
    auto& mov = ecs::get<movie_t>(e);
    mov.playing = false;
    mov.time = static_cast<float>(frame);
    mov.trunc_time();
    apply_frame(e, mov);
}


/*** EASING CALCULATION ***/

// math is not hard, but code has been stolen from precious web,
// look for `fl.motion`, BezierEase, BezierSegment, CustomEase

int get_quadratic_roots(float* out_roots, float a, float b, float c) {
// make sure we have a quadratic
    if (a == 0.0f) {
        if (b == 0.0f) {
            return 0;
        }
        out_roots[0] = -c / b;
        return 1;
    }

    const float q = b * b - 4 * a * c;

    if (q > 0.0f) {
        const float c = sqrt(q) / (2 * a);
        const float d = -b / (2 * a);
        out_roots[0] = d - c;
        out_roots[1] = d + c;
        return 2;
    } else if (q < 0.0f) {
        return 0;
    }
    out_roots[0] = -b / (2 * a);
    return 1;
}

int get_cubic_roots(float* out_roots, float a = 0.0f, float b = 0.0f, float c = 0.0f, float d = 0.0f) {
    // make sure we really have a cubic
    if (a == 0.0f) {
        return get_quadratic_roots(out_roots, b, c, d);
    }

    // normalize the coefficients so the cubed term is 1 and we can ignore it hereafter
    b /= a;
    c /= a;
    d /= a;

    const float q = (b * b - 3 * c) / 9;               // won't change over course of curve
    const float q_cubed = q * q * q;                  // won't change over course of curve
    const float r = (2 * b * b * b - 9 * b * c + 27 * d) / 54; // will change because d changes
    // but parts with b and c won't change
    // determine if there are 1 or 3 real roots using r and q
    const float diff = q_cubed - r * r;
    if (diff >= 0.0f) {
        // avoid division by zero
        if (q == 0.0f) {
            out_roots[0] = 0.0f;
            return 1;
        }

        // three real roots
        const float theta = acos(r / sqrt(q_cubed)); // will change because r changes
        const float q_sqrt = sqrt(q); // won't change

        out_roots[0] = -2 * q_sqrt * cos(theta / 3) - b / 3;
        out_roots[1] = -2 * q_sqrt * cos((theta + 2 * M_PI) / 3) - b / 3;
        out_roots[2] = -2 * q_sqrt * cos((theta + 4 * M_PI) / 3) - b / 3;

        return 3;
    }
    // one real root
    const float tmp = pow(sqrt(-diff) + abs(r), 1.0f / 3.0f);
    const float r_sign = r > 0.0f ? 1.0f : (r < 0.0f ? -1.0f : 0.0f);
    out_roots[0] = -r_sign * (tmp + q / tmp) - b / 3;
    return 1;
}

float get_bezier_value_normalized(float t, float a, float b, float c, float d) {
    return (t * t * (d - a) + 3 * (1 - t) * (t * (c - a) + (1 - t) * (b - a))) * t + a;
}

float get_bezier_y(const float2* curve, float x) {
    const float eps = 0.000001f;
    const float a = curve[0].x;
    const float b = curve[1].x;
    const float c = curve[2].x;
    const float d = curve[3].x;

    if (a < d) {
        if (x <= a + eps) return curve[0].y;
        if (x >= d - eps) return curve[3].y;
    } else {
        if (x >= a + eps) return curve[0].y;
        if (x <= d - eps) return curve[3].y;
    }

    const float coeffs[4] = {
            -a + 3 * b - 3 * c + d,
            3 * a - 6 * b + 3 * c,
            -3 * a + 3 * b,
            a
    };

    // x(t) = a*t^3 + b*t^2 + c*t + d
    float roots[3] = {};
    int roots_count = get_cubic_roots(roots, coeffs[0], coeffs[1], coeffs[2], coeffs[3] - x);
    float time = 0.0f;
    for (int i = 0; i < roots_count; ++i) {
        const float r = roots[i];
        if (0.0f <= r && r <= 1.0f) {
            time = r;
            break;
        }
    }

    return get_bezier_value_normalized(time,
                                       curve[0].y,
                                       curve[1].y,
                                       curve[2].y,
                                       curve[3].y);
}

float ease(float x, const easing_data_t& data) {
    float y = x;
    if (data.curve.size() > 3) {
        // bezier
        for (size_t i = 0; i < data.curve.size() - 3; ++i) {
            const float x0 = data.curve[i].x;
            const float x1 = data.curve[i + 3].x;
            if (x0 <= x && x <= x1) {
                return get_bezier_y(data.curve.data() + i, x);
            }
        }
    } else if (data.ease != 0) {
        float e = data.ease; //  / 100.0f
        float t;
        if (e < 0.0f) {
            // Ease in
            float inv = 1.0f - x;
            t = 1.0f - inv * inv;
            e = -e;
        } else {
            // Ease out
            t = x * x;
        }
        y = e * t + (1.0f - e) * x;
    }
    return y;
}

}
