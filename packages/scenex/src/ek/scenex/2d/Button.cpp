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
//
//void initialize_events(entity_t e) {
//    auto& eh = ecs::add<NodeEventHandler>(e);
//    eh.on(POINTER_EVENT_OVER, [](const NodeEventData& ev) {
//        auto& btn = ecs::get<Button>(ev.receiver);
//        const auto& skin = get_skin(btn);
//        play_button_sound(skin.sfxOver);
//    });
//    eh.on(POINTER_EVENT_OUT, [](const NodeEventData& ev) {
//        auto& btn = ecs::get<Button>(ev.receiver);
//        const auto& skin = get_skin(btn);
//        if (ecs::get<Interactive>(ev.receiver).pushed) {
//            start_post_tween(btn);
//            play_button_sound(skin.sfxCancel);
//        } else {
//            play_button_sound(skin.sfxOut);
//        }
//    });
//    eh.on(POINTER_EVENT_DOWN, [](const NodeEventData& ev) {
//        auto& btn = ecs::get<Button>(ev.receiver);
//        const auto& skin = get_skin(btn);
//        play_button_sound(skin.sfxDown);
//    });
//    eh.on(POINTER_EVENT_TAP, [](const NodeEventData& ev) {
//        auto& btn = ecs::get<Button>(ev.receiver);
//        const auto& skin = get_skin(btn);
//        auto* ev_eh = ecs::try_get<NodeEventHandler>(ev.receiver);
//        if (ev_eh) {
//            ev_eh->emit({BUTTON_EVENT_CLICK, ev.source, {nullptr}, ev.source});
//        }
//
//        play_button_sound(skin.sfxClick);
//        start_post_tween(btn);
//
//        // TODO:
//        //auto name = ecs::get_or_default<NodeName>(e).name;
//        //if (!name.empty()) {
//        //    analytics::event("click", name.c_str());
//        //}
//    });
//
//    eh.on(INTERACTIVE_EVENT_BACK_BUTTON, [](const NodeEventData& ev) {
//        auto& btn = ecs::get<Button>(ev.receiver);
//        if (btn.back_button) {
//            auto* ev_eh = ecs::try_get<NodeEventHandler>(ev.receiver);
//            if (ev_eh) {
//                ev_eh->emit({BUTTON_EVENT_CLICK, ev.receiver, {nullptr}, ev.receiver});
//            }
//            start_post_tween(btn);
//            ev.processed = true;
//        }
//    });
//}

void apply_skin(const ButtonSkin& skin, const Button& btn, Transform2D& transform) {
    // TODO: skin params?
    (void) skin;

    const float over = btn.timeOver;
    const float push = btn.timePush;
    const float post = btn.timePost;
    const float pi = MATH_PI;

    float sx = 1.0f + 0.2f * sinf((1.0f - post) * pi * 5.0f) * post;
    float sy = 1.0f + 0.2f * sinf((1.0f - post) * pi) * cosf((1.0f - post) * pi * 5.0f) * post;

    transform.set_scale(btn.baseScale * vec2(sx, sy));

    const auto color = lerp_color(COLOR_WHITE, ARGB(0xFF888888), push);
    transform.color.scale = mul_color(btn.baseColor.scale, color);

    const float h = 0.1f * over;
    transform.color.offset = btn.baseColor.offset + color_4f(h, h, h, 0);
}

void update_movie_frame(entity_t entity, const Interactive& interactive) {
    MovieClip* mc = ecs::try_get<MovieClip>(entity);
    if (mc) {
        int frame = 0;
        if (interactive.over || interactive.pushed) {
            frame = 1;
            if (interactive.pushed && interactive.over) {
                frame = 2;
            }
        }
        goto_and_stop(entity, (float)frame);
    }
}

void update_button_events(Interactive* interactive, Button* button, entity_t e, entity_t* queue, uint32_t* queue_num) {
    if(interactive->ev_over) {
        const auto& skin = get_skin(*button);
        play_button_sound(skin.sfxOver);
    }
    if(interactive->ev_out) {
        const auto& skin = get_skin(*button);
        if(button->pushed) {
            start_post_tween(*button);
            play_button_sound(skin.sfxCancel);
        }
        else {
            play_button_sound(skin.sfxOut);
        }
        button->pushed = false;
    }
    if(interactive->ev_down) {
        const auto& skin = get_skin(*button);
        play_button_sound(skin.sfxDown);
    }
    if(interactive->ev_tap) {
        auto* ev_eh = ecs::try_get<NodeEventHandler>(e);
        if (ev_eh) {
            queue[*queue_num] = e;
            ++(*queue_num);
            //ev_eh->emit({BUTTON_EVENT_CLICK, e, {nullptr}, e});
        }
        const auto& skin = get_skin(*button);
        play_button_sound(skin.sfxClick);
        start_post_tween(*button);
    }
    button->pushed = interactive->pushed;
}

void Button::updateAll() {
    FixedArray<entity_t, 64> tap_events;
    for(auto e : ecs::view<Button>()) {
        Button* btn = ecs::try_get<Button>(e);
        Interactive* interactive = ecs::try_get<Interactive>(e);
        Transform2D* transform = ecs::try_get<Transform2D>(e);
        if(interactive && transform) {
            float dt = g_time_layers[btn->time].dt;

            if (!btn->initialized) {
                btn->initialized = true;

                initialize_base_transform(*btn, *transform);
                //initialize_events(e);
            }

            update_button_events(interactive, btn, e, tap_events.data(), &tap_events._size);

            const auto& skin = get_skin(*btn);

            btn->timeOver = reach_delta(btn->timeOver,
                                       interactive->over ? 1.0f : 0.0f,
                                       dt * skin.overSpeedForward,
                                       -dt * skin.overSpeedBackward);

            btn->timePush = reach_delta(btn->timePush,
                                       interactive->pushed ? 1.0f : 0.0f,
                                       dt * skin.pushSpeedForward,
                                       -dt * skin.pushSpeedBackward);

            btn->timePost = reach(btn->timePost, 0.0f, 2.0f * dt);

            apply_skin(skin, *btn, *transform);
            update_movie_frame(e, *interactive);
        }
    }

    for(auto e : tap_events) {
        if(is_entity(e)) {
            auto* ev_eh = ecs::try_get<NodeEventHandler>(e);
            if (ev_eh) {
                ev_eh->emit({BUTTON_EVENT_CLICK, e, {nullptr}, e});
            }
        }
    }
}
}
