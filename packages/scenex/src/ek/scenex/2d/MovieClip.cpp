#include "MovieClip.hpp"
#include "Transform2D.hpp"

#include <ek/scenex/base/Node.hpp>

namespace ek {

using ecs::EntityApi;

float ease(float x, const SGEasingData& data);

void apply_frame(EntityApi e, MovieClip& mov);

void MovieClip::updateAll() {
    for (auto e : ecs::view<MovieClip>()) {
        auto& mov = e.get<MovieClip>();
        auto dt = mov.timer->dt;
        if (mov.playing) {
            mov.time += dt * mov.fps;
            mov.trunc_time();
            apply_frame(e, mov);
        }
    }
}

int findKeyFrame(const Array<SGMovieFrameData>& frames, float t) {
    const int end = int(frames.size());
    for (int i = 0; i < end; ++i) {
        const auto& kf = frames[i];
        if (t >= kf.index && t < kf.index + kf.duration) {
            return i;
        }
    }
    return -1;
}

struct easing_progress_t {
    float position;
    float scale;
    float skew;
    float color;

    void fill(float t) {
        position = scale = skew = color = t;
    }
};

SGKeyFrameTransform lerp(const SGKeyFrameTransform& begin,
                         const SGKeyFrameTransform& end,
                         const easing_progress_t& progress) {
    return {
            lerp_vec2(begin.position, end.position, progress.position),
            lerp_vec2(begin.scale, end.scale, progress.scale),
            lerp_vec2(begin.skew, end.skew, progress.skew),
            begin.pivot,
            lerp_color2f(begin.color, end.color, progress.color)
    };
}

easing_progress_t get_easing_progress(const float t, const Array<SGEasingData>& easing) {
    easing_progress_t progress{};
    if (easing.empty()) {
        progress.fill(t);
        return progress;
    }

    for (const auto& e : easing) {
        const float x = ease(t, e);
        if (e.attribute == 0) {
            progress.fill(x);
            break;
        } else if (e.attribute == 1) {
            progress.position = x;
        } else if (e.attribute == 2) {
            progress.skew = x;
        } else if (e.attribute == 3) {
            progress.scale = x;
        } else if (e.attribute == 4) {
            progress.color = x;
        }
    }
    return progress;
}

void apply_transform(EntityApi e, const SGKeyFrameTransform& keyframe) {
    auto& transform = e.get_or_create<Transform2D>();
    transform.setTransform(keyframe.position, keyframe.scale, keyframe.skew, keyframe.pivot);
    transform.color.scale = color_vec4(keyframe.color.scale);
    transform.color.offset = color_vec4(keyframe.color.offset);
}

void update_target(float time, EntityApi e, const SGMovieLayerData& layer) {
    auto& config = e.get_or_create<Node>();
    const auto ki = findKeyFrame(layer.frames, time);
    if (ki < 0) {
        config.setVisible(false);
        return;
    }
    const auto& k1 = layer.frames[ki];
    config.setVisible(k1.visible);
    if (k1.motion_type == 1 && (ki + 1) < layer.frames.size()) {
        const auto& k2 = layer.frames[ki + 1];
        const float t = k1.getLocalTime(time);
        const auto progress = get_easing_progress(t, k1.easing);
        const auto keyframe = lerp(k1.transform, k2.transform, progress);
        apply_transform(e, keyframe);
    } else {
        apply_transform(e, k1.transform);
    }

    if (k1.loopMode != 0 && e.has<MovieClip>()) {
        auto& mc = e.get<MovieClip>();
        const auto loop = k1.loopMode;
        if (loop == 1) {
            goto_and_stop(e, time - k1.index);
        } else if (loop == 2) {
            const auto offset = fmin(time, k1.index + k1.duration) - k1.index;
            auto t = k1.firstFrame + offset;
            const auto* mcData = mc.get_movie_data();
            if (mcData && t > mcData->frames) {
                t = mcData->frames;
            }
            goto_and_stop(e, t);
        } else if (loop == 3) {
            goto_and_stop(e, k1.firstFrame);
        }
    }
}

void apply_frame(EntityApi e, MovieClip& mov) {
    auto* data = mov.get_movie_data();
    auto time = mov.time;
    if (!data) {
        // no data - exit early
        return;
    }
    auto it = e.get<Node>().child_first;
    const auto totalTargets = static_cast<int>(data->layers.size());
    while (it != nullptr) {
        if (it.has<MovieClipTargetIndex>()) {
            const auto idx = it.get<MovieClipTargetIndex>().key;
            if (idx < totalTargets) {
                update_target(time, it, data->layers[idx]);
            }
        }
        it = it.get<Node>().sibling_next;
    }
}

void goto_and_stop(EntityApi e, float frame) {
    auto& mov = e.get<MovieClip>();
    mov.playing = false;
    mov.time = frame;
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

        out_roots[0] = -2 * q_sqrt * cos(theta / 3.0f) - b / 3.0f;
        out_roots[1] = -2 * q_sqrt * cos((theta + MATH_TAU) / 3.0f) - b / 3.0f;
        out_roots[2] = -2 * q_sqrt * cos((theta + 4 * MATH_PI) / 3.0f) - b / 3.0f;

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

float get_bezier_y(const vec2_t* curve, float x) {
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

float ease(float x, const SGEasingData& data) {
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
