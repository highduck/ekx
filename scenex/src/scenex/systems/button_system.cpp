#include "button_system.h"
#include <scenex/components/interactive_t.h>
#include <ek/math/common.hpp>
#include <ek/math/rand.hpp>
#include <scenex/components/transform_2d.h>
#include <scenex/components/movie_t.h>
#include <scenex/systems/movie_clip_system.h>
#include <scenex/components/name_t.h>
#include <scenex/simple_audio_manager.h>
#include <ek/locator.hpp>
#include <platform/analytics.hpp>
#include <scenex/components/event_handler.h>

using ek::operator ""_argb;
using namespace ek::math;

namespace scenex {

void play_sound(const std::string& id) {
    ek::resolve<simple_audio_manager>().play_sound(id);
}

void start_post_tween(button_t& btn) {
    btn.post_time = fmaxf(ek::random(0.7f, 1.0f), btn.post_time);
}

void handle_back_button(button_t& btn, const event_data& ev) {
    if (btn.back_button) {
        btn.clicked.emit();
        start_post_tween(btn);
        ev.processed = true;
    }
}

void initialize_base_transform(button_t& btn, const transform_2d& transform) {
    btn.base_color_multiplier = transform.colorMultiplier;
    btn.base_color_offset = transform.colorOffset;
    btn.base_scale = transform.scale;
    btn.base_skew = transform.skew;
}

const button_skin& get_skin(const button_t& btn) {
    static button_skin basic_skin{};
    return btn.skin ? *btn.skin : basic_skin;
}

void initialize_events(ecs::entity e) {
    auto& interactive = ecs::get_or_create<interactive_t>(e);
    interactive.on_over.add([e] {
        const auto& skin = get_skin(ecs::get<button_t>(e));
        play_sound(skin.sfxOver);
    });
    interactive.on_out.add([e] {
        auto& btn = ecs::get<button_t>(e);
        const auto& skin = get_skin(btn);
        if (ecs::get<interactive_t>(e).pushed) {
            start_post_tween(btn);
        }
        play_sound(skin.sfxOut);
    });
    interactive.on_down.add([e] {
        const auto& skin = get_skin(ecs::get<button_t>(e));
        play_sound(skin.sfxDown);
    });
    interactive.on_clicked.add([e] {
        auto& btn = ecs::get<button_t>(e);
        const auto& skin = get_skin(btn);
        play_sound(skin.sfxClick);

        start_post_tween(btn);
        btn.clicked.emit();
        auto name = ecs::get_or_default<name_t>(e).name;
        if (!name.empty()) {
            ek::analytics::event("click", name.c_str());
        }
    });

    ecs::get_or_create<event_handler_t>(e).on("backButton", [e](const event_data& ev) {
        auto& btn = ecs::get<button_t>(e);
        handle_back_button(btn, ev);
    });

}

void apply_skin(const button_skin& skin, const button_t& btn, transform_2d& transform) {
    const float over = btn.over_time;
    const float push = btn.push_time;
    const float post = btn.post_time;

    float sx = 1.0f + 0.5f * sinf((1.0f - post) * pi * 5.0f) * post;
    float sy = 1.0f + 0.5f * sinf((1.0f - post) * pi) * cosf((1.0f - post) * pi * 5.0f) * post;

    auto color = lerp(0xFFFFFFFF_argb, 0xFF888888_argb, push);
    transform.colorMultiplier = btn.base_color_multiplier * color;

    const float h = 0.1f * over;
    transform.colorOffset = btn.base_color_offset + argb32_t{h, h, h, 0.0f};
    transform.scale = btn.base_scale * float2(sx, sy);
}

void update_movie_frame(ecs::entity entity, const interactive_t& interactive) {
    if (ecs::has<movie_t>(entity)) {
        int frame = 0;
        if (interactive.over || interactive.pushed) {
            frame = 1;
            if (interactive.pushed && interactive.over) {
                frame = 2;
            }
        }
        goto_and_stop(entity, frame);
    }
}

void update_buttons(float dt) {

    for (auto e : ecs::view<button_t, interactive_t, transform_2d>()) {
        auto& btn = ecs::get<button_t>(e);
        auto& interactive = ecs::get<interactive_t>(e);
        auto& transform = ecs::get<transform_2d>(e);

        if (!btn.initialized) {
            btn.initialized = true;
            interactive.cursor = interactive_t::mouse_cursor_t::button;
            initialize_base_transform(btn, transform);
            initialize_events(e);
        }

        const auto& skin = get_skin(btn);

        btn.over_time = reach_delta(btn.over_time,
                                    interactive.over ? 1.0f : 0.0f,
                                    dt * skin.overSpeedForward,
                                    -dt * skin.overSpeedBackward);

        btn.push_time = reach_delta(btn.push_time,
                                    interactive.pushed ? 1.0f : 0.0f,
                                    dt * skin.pressSpeedForward,
                                    -dt * skin.pressSpeedBackward);

        btn.post_time = reach(btn.post_time, 0.0f, 2.0f * dt);

        apply_skin(skin, btn, transform);
        update_movie_frame(e, interactive);
    }
}
}
