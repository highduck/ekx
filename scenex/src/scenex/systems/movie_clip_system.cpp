#include "movie_clip_system.h"

#include "game_time.h"
#include <scenex/components/transform_2d.h>

namespace scenex {

using ecs::entity;

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
            float progress = static_cast<float>(time - k1.index) / k1.duration;
            if (k1.ease != 0) {
                float e = k1.ease / 100.0f;
                float t;
                if (e < 0.0f) {
                    // Ease in
                    float inv = 1.0f - progress;
                    t = 1.0f - inv * inv;
                    e = -e;
                } else {
                    // Ease out
                    t = progress * progress;
                }
                progress = e * t + (1.0f - e) * progress;
            }

            auto p = lerp(k1.position, k2.position, progress);
            auto& transform = ecs::get<transform_2d>(target);
            transform.matrix.tx = p.x;
            transform.matrix.ty = p.y;
            transform.skew = lerp(k1.skew, k2.skew, progress);
            transform.scale = lerp(k1.scale, k2.scale, progress);
            transform.colorMultiplier = argb32_t{
                    lerp(k1.color.multiplier, k2.color.multiplier, progress)
            };
            transform.colorOffset = argb32_t{
                    lerp(k1.color.offset, k2.color.offset, progress)
            };

            auto m = transform.matrix;
            m.set(p, transform.scale, transform.skew);
            m.tx = m.tx - m.a * k1.pivot.x - m.c * k1.pivot.y;
            m.ty = m.ty - m.b * k1.pivot.x - m.d * k1.pivot.y;
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

}
