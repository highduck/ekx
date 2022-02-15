#include "PopupManager.hpp"
#include <ek/canvas.h>

#include <ek/scenex/base/DestroyTimer.hpp>
#include <ek/scenex/SceneFactory.hpp>
#include <ek/scenex/base/Interactive.hpp>
#include <ek/scenex/InteractionSystem.hpp>
#include <ek/scenex/2d/Button.hpp>
#include <ek/scenex/2d/LayoutRect.hpp>


#include <ek/scenex/base/Tween.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/base/NodeEvents.hpp>

namespace ek {

using namespace ecs;

const float tweenDelay = 0.05f;
const float tweenDuration = 0.3f;
const float animationVertDistance = 200.0f;

void on_popup_pause(entity_t e) {
    set_touchable(e, false);
}

void on_popup_resume(entity_t e) {
    set_touchable(e, true);
}

void on_popup_opening(entity_t e) {
    set_scale(e, vec2(0,0));
    set_touchable(e, false);
    set_visible(e, true);
}

void on_popup_open_animation(float t, entity_t e) {
    t = saturate(t);
    const float scale = ease_back_out(t);
    const float fly = ease_p3_out(t);
    set_scale_f(e, scale);
    set_y(e, animationVertDistance * (1.0f - fly));
}

void on_popup_opened(entity_t e) {
    set_touchable(e, true);
}

void on_popup_closing(entity_t e) {
    set_touchable(e, false);
}

void on_popup_closed(entity_t e) {
    entity_t* ptr = g_popup_manager.active.find(e);
    if (ptr) {
        g_popup_manager.active.erase_ptr(ptr);
    }

    if (g_popup_manager.active.empty()) {
        entity_t pm = g_popup_manager.entity;
        set_touchable(pm, false);
        set_visible(pm, false);
    } else {
        on_popup_resume(entity_t{g_popup_manager.active.back()});
    }

    set_visible(e, false);
    // TODO: flag auto-delete
    destroy_later(e);
}

void on_popup_close_animation(float t, entity_t e) {
    t = saturate(1 - t);
    const float scale = ease_back_out(t);
    const float fly = ease_p3_out(t);
    set_scale_f(e, scale);
    set_y(e, animationVertDistance * (fly - 1.0f));
}

void init_basic_popup(entity_t e) {
    auto node_close = find(e, H("btn_close"));
    if (node_close.id) {
        auto* i = ecs::try_get<Interactive>(node_close);
        if(i) {
            i->back_button = true;
            auto* btn = ecs::try_get<Button>(node_close);
            if (btn) {
                ecs::add<NodeEventHandler>(node_close).on(BUTTON_EVENT_CLICK, [](const NodeEventData& event) {
                    close_popup(get_parent(event.receiver));
                });
            }
        }
    }
}

void open_popup(entity_t e) {
    g_popup_manager.closing_last = NULL_ENTITY;

    // if we have entity in active list - do nothing
    if (g_popup_manager.active.find(e) != nullptr) {
        return;
    }

    if (ecs::get<Node>(e).parent.id == g_popup_manager.layer.id) {
        return;
    }

    if (!g_popup_manager.active.empty()) {
        on_popup_pause(g_popup_manager.active.back());
    }
    g_popup_manager.active.push_back(e);
    on_popup_opening(e);

    auto& tween = Tween::reset(e);
    tween.delay = tweenDelay;
    tween.duration = tweenDuration;
    tween.advanced = [](entity_t e, float r) {
        if (r >= 1.0f) {
            on_popup_opened(e);
        }
        on_popup_open_animation(r, e);
    };

    append(g_popup_manager.layer, e);
    set_touchable(g_popup_manager.entity, true);
    set_visible(g_popup_manager.entity, true);
}

void close_popup(entity_t e) {
    // we cannot close entity if it is not active
    if (g_popup_manager.active.find(e) == nullptr) {
        return;
    }

    if (g_popup_manager.active.back().id == e.id) {
        g_popup_manager.closing_last = e;
    }
    on_popup_closing(e);

    auto& tween = Tween::reset(e);
    tween.delay = tweenDelay;
    tween.duration = tweenDuration;
    tween.advanced = [](entity_t e, float t) {
        on_popup_close_animation(t, e);
        if (t >= 1.0f) {
            on_popup_closed(e);
        }
    };
}


uint32_t count_active_popups() {
    return g_popup_manager.active.size();
}

void clear_popups() {

    g_popup_manager.closing_last = NULL_ENTITY;
    g_popup_manager.fade_progress = 0.0f;
    set_alpha(g_popup_manager.back, 0);

    destroy_children(g_popup_manager.layer);
    g_popup_manager.active.clear();

    const entity_t e = g_popup_manager.entity;
    set_touchable(e, false);
    set_visible(e, false);
}

void close_all_popups() {
    auto copy_vec = g_popup_manager.active;
    for (auto p: copy_vec) {
        close_popup(p);
    }
}

entity_t createBackQuad() {
    auto e = createNode2D(H("back"));
    auto& display = ecs::add<Display2D>(e);
    quad2d_setup(e)->setColor(COLOR_BLACK);
    display.program = R_SHADER_SOLID_COLOR;
    ecs::add<LayoutRect>(e)
            .fill(true, true)
            .setInsetsMode(false);

    // intercept back-button if popup manager is active
    ecs::add<Interactive>(e);
    auto& eh = ecs::add<NodeEventHandler>(e);
    eh.on(INTERACTIVE_EVENT_BACK_BUTTON, [](const NodeEventData& ev) {
        ev.processed = true;
    });

    // if touch outside of popups, simulate back-button behavior
    eh.on(POINTER_EVENT_DOWN, [](auto) {
        if (!g_popup_manager.active.empty()) {
            g_interaction_system.sendBackButton();
        }
    });

    return e;
}

}

ek::PopupManager g_popup_manager;

void popup_manager_init() {
    using namespace ek;

    g_popup_manager.fade_progress = 0.0f;
    g_popup_manager.fade_duration = 0.3f;
    g_popup_manager.fade_alpha = 0.5f;

    auto e = createNode2D(H("popups"));
    g_popup_manager.entity = e;
    auto& pm = g_popup_manager;
    pm.back = createBackQuad();
    append(e, pm.back);

    pm.layer = createNode2D(H("layer"));
    ecs::add<LayoutRect>(pm.layer)
            .enableAlignX(0.5f)
            .enableAlignY(0.5f)
            .setInsetsMode(false);
    append(e, pm.layer);

    // initially popup manager is deactivated
    set_touchable(e, false);
    set_visible(e, false);
}

void update_popup_manager() {
    using namespace ek;
    auto& p = g_popup_manager;
    if(UNLIKELY(!p.entity.id)) {
        return;
    }
    auto dt = g_time_layers[TIME_LAYER_UI].dt;
    bool need_fade = !p.active.empty();
    if (p.active.size() == 1 && p.active.back().id == p.closing_last.id) {
        need_fade = false;
    }
    p.fade_progress = reach(p.fade_progress,
                            need_fade ? 1.0f : 0.0f,
                            dt / p.fade_duration);

    set_alpha_f(p.back, p.fade_alpha * p.fade_progress);

//    if (!p.active.empty()) {
//        auto front = p.active.back();
//        if (ecs::has<close_timeout>(front)) {
//            auto& t = ecs::get<close_timeout>(front);
//            t.time -= dt;
//            if (t.time <= 0.0f) {
//                ecs::remove<close_timeout>(front);
//                close_popup(front);
//            }
//        }
//    }
}

