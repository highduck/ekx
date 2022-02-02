#include "Button.hpp"

#include <ek/scenex/base/Interactive.hpp>
#include <ek/math.h>
#include <ek/rnd.h>
#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/2d/MovieClip.hpp>
#include <ekx/app/audio_manager.h>

//#include <ek/firebase/Firebase.h>
#include <ek/scenex/base/NodeEvents.hpp>
#include <ek/scenex/InteractionSystem.hpp>

namespace ek {

static inline void play_button_sound(string_hash_t id) {
    play_sound(id);
}

void start_post_tween(Button& btn) {
    btn.timePost = fmaxf(random_range_f(0.7f, 1.0f), btn.timePost);
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

void initialize_events(ecs::EntityApi e) {
    auto& eh = e.get_or_create<NodeEventHandler>();
    eh.on(POINTER_EVENT_OVER, [](const NodeEventData& ev) {
        auto& btn = ecs::EntityApi{ev.source}.get<Button>();
        const auto& skin = get_skin(btn);
        play_button_sound(skin.sfxOver);
    });
    eh.on(POINTER_EVENT_OUT, [](const NodeEventData& ev) {
        auto& btn = ecs::EntityApi{ev.source}.get<Button>();
        const auto& skin = get_skin(btn);
        if (ecs::EntityApi{ev.source}.get<Interactive>().pushed) {
            start_post_tween(btn);
            play_button_sound(skin.sfxCancel);
        } else {
            play_button_sound(skin.sfxOut);
        }
    });
    eh.on(POINTER_EVENT_DOWN, [](const NodeEventData& ev) {
        auto& btn = ecs::EntityApi{ev.source}.get<Button>();
        const auto& skin = get_skin(btn);
        play_button_sound(skin.sfxDown);
    });
    eh.on(POINTER_EVENT_TAP, [](const NodeEventData& ev) {
        auto& btn = ecs::EntityApi{ev.source}.get<Button>();
        const auto& skin = get_skin(btn);
        auto* ev_eh = ecs::EntityApi{ev.source}.tryGet<NodeEventHandler>();
        if (ev_eh) {
            ev_eh->emit({BUTTON_EVENT_CLICK, ev.source});
        }

        play_button_sound(skin.sfxClick);
        start_post_tween(btn);

        // TODO:
        //auto name = e.get_or_default<NodeName>().name;
        //if (!name.empty()) {
        //    analytics::event("click", name.c_str());
        //}
    });

    eh.on(INTERACTIVE_EVENT_BACK_BUTTON, [current = e](const NodeEventData& ev) {
        auto& btn = current.get<Button>();
        if (btn.back_button) {
            auto* ev_eh = current.tryGet<NodeEventHandler>();
            if (ev_eh) {
                ev_eh->emit({BUTTON_EVENT_CLICK, current.index});
            }
            start_post_tween(btn);
            ev.processed = true;
        }
    });
}

void apply_skin(const ButtonSkin& skin, const Button& btn, Transform2D& transform) {
    // TODO: skin params?
    (void) skin;

    const float over = btn.timeOver;
    const float push = btn.timePush;
    const float post = btn.timePost;
    const float pi = MATH_PI;

    float sx = 1.0f + 0.2f * sinf((1.0f - post) * pi * 5.0f) * post;
    float sy = 1.0f + 0.2f * sinf((1.0f - post) * pi) * cosf((1.0f - post) * pi * 5.0f) * post;

    transform.setScale(btn.baseScale * vec2(sx, sy));

    const auto color = lerp_color(COLOR_WHITE, ARGB(0xFF888888), push);
    transform.color.scale = mul_color(btn.baseColor.scale, color);

    const float h = 0.1f * over;
    transform.color.offset = btn.baseColor.offset + color_4f(h, h, h, 0);
}

void update_movie_frame(ecs::EntityApi entity, const Interactive& interactive) {
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
    for (auto e: ecs::view<Button, Interactive, Transform2D>()) {
        auto& btn = e.get<Button>();
        auto& interactive = e.get<Interactive>();
        auto& transform = e.get<Transform2D>();
        float dt = g_time_layers[btn.time].dt;

        if (!btn.initialized) {
            btn.initialized = true;
            interactive.cursor = EK_MOUSE_CURSOR_BUTTON;
            initialize_base_transform(btn, transform);
            initialize_events(e);
        }

        const auto& skin = get_skin(btn);

        btn.timeOver = reach_delta(btn.timeOver,
                                   interactive.over ? 1.0f : 0.0f,
                                   dt * skin.overSpeedForward,
                                   -dt * skin.overSpeedBackward);

        btn.timePush = reach_delta(btn.timePush,
                                   interactive.pushed ? 1.0f : 0.0f,
                                   dt * skin.pushSpeedForward,
                                   -dt * skin.pushSpeedBackward);

        btn.timePost = reach(btn.timePost, 0.0f, 2.0f * dt);

        apply_skin(skin, btn, transform);
        update_movie_frame(e, interactive);
    }
}
}
