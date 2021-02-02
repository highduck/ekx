#include "Button.hpp"

#include <ek/scenex/base/Interactive.hpp>
#include <ek/math/common.hpp>
#include <ek/math/rand.hpp>
#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/2d/MovieClip.hpp>
#include <ek/scenex/AudioManager.hpp>
#include <ek/util/locator.hpp>
#include <ek/ext/analytics/analytics.hpp>
#include <ek/scenex/base/NodeEvents.hpp>
#include <ek/scenex/InteractionSystem.hpp>
#include <ek/scenex/base/Node.hpp>

namespace ek {

void play_sound(const std::string& id) {
    resolve<AudioManager>().play_sound(id);
}

void start_post_tween(Button& btn) {
    btn.post_time = fmaxf(random(0.7f, 1.0f), btn.post_time);
}

void handle_back_button(Button& btn, const NodeEventData& ev) {
    if (btn.back_button) {
        btn.clicked.emit();
        start_post_tween(btn);
        ev.processed = true;
    }
}

void initialize_base_transform(Button& btn, const Transform2D& transform) {
    btn.baseColor = transform.color;
    btn.baseScale = transform.getScale();
    btn.baseSkew = transform.getSkew();
}

const ButtonSkin& get_skin(const Button& btn) {
    static ButtonSkin basic_skin{};
    return btn.skin ? *btn.skin : basic_skin;
}

void initialize_events(ecs::entity e) {
    auto& interactive = e.get_or_create<Interactive>();
    interactive.on_over.add([e] {
        const auto& skin = get_skin(e.get<Button>());
        play_sound(skin.sfx_over);
    });
    interactive.on_out.add([e] {
        auto& btn = e.get<Button>();
        const auto& skin = get_skin(btn);
        if (e.get<Interactive>().pushed) {
            start_post_tween(btn);
        }
        play_sound(skin.sfx_out);
    });
    interactive.on_down.add([e] {
        const auto& skin = get_skin(e.get<Button>());
        play_sound(skin.sfx_down);
    });
    interactive.on_clicked.add([e] {
        auto& btn = e.get<Button>();
        const auto& skin = get_skin(btn);
        play_sound(skin.sfx_click);

        start_post_tween(btn);
        btn.clicked.emit();
        auto name = e.get_or_default<NodeName>().name;
        if (!name.empty()) {
            analytics::event("click", name.c_str());
        }
    });

    e.get_or_create<NodeEventHandler>()
            .on(interactive_event::back_button, [e](const NodeEventData& ev) {
                auto& btn = e.get<Button>();
                handle_back_button(btn, ev);
            });

}

void apply_skin(const ButtonSkin& skin, const Button& btn, Transform2D& transform) {
    const float over = btn.over_time;
    const float push = btn.push_time;
    const float post = btn.post_time;
    const float pi = math::pi;

    float sx = 1.0f + 0.2f * sinf((1.0f - post) * pi * 5.0f) * post;
    float sy = 1.0f + 0.2f * sinf((1.0f - post) * pi) * cosf((1.0f - post) * pi * 5.0f) * post;

    transform.setScale(btn.baseScale * float2(sx, sy));

    auto color = lerp(0xFFFFFFFF_argb, 0xFF888888_argb, push);
    transform.color.scale = btn.baseColor.scale * color;

    const float h = 0.1f * over;
    transform.color.offset = btn.baseColor.offset + argb32_t{h, h, h, 0.0f};
}

void update_movie_frame(ecs::entity entity, const Interactive& interactive) {
    if (entity.has<MovieClip>()) {
        int frame = 0;
        if (interactive.over || interactive.pushed) {
            frame = 1;
            if (interactive.pushed && interactive.over) {
                frame = 2;
            }
        }
        goto_and_stop(entity, static_cast<float>(frame));
    }
}

void Button::updateAll() {

    for (auto e : ecs::view<Button, Interactive, Transform2D>()) {
        auto& btn = e.get<Button>();
        auto& interactive = e.get<Interactive>();
        auto& transform = e.get<Transform2D>();
        float dt = btn.time->dt;

        if (!btn.initialized) {
            btn.initialized = true;
            interactive.cursor = Interactive::mouse_cursor::button;
            initialize_base_transform(btn, transform);
            initialize_events(e);
        }

        const auto& skin = get_skin(btn);

        btn.over_time = math::reach_delta(btn.over_time,
                                          interactive.over ? 1.0f : 0.0f,
                                          dt * skin.over_speed_forward,
                                          -dt * skin.over_speed_backward);

        btn.push_time = math::reach_delta(btn.push_time,
                                          interactive.pushed ? 1.0f : 0.0f,
                                          dt * skin.push_speed_forward,
                                          -dt * skin.push_speed_backward);

        btn.post_time = math::reach(btn.post_time, 0.0f, 2.0f * dt);

        apply_skin(skin, btn, transform);
        update_movie_frame(e, interactive);
    }
}
}
