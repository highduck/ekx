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

void on_popup_pause(EntityApi e) {
    setTouchable(e, false);
}

void on_popup_resume(EntityApi e) {
    setTouchable(e, true);
}

void on_popup_opening(EntityApi e) {
    e.get<Transform2D>().setScale(0);
    e.get<Node>().setTouchable(false);
    e.get<Node>().setVisible(true);
}

void on_popup_open_animation(float t, EntityApi e) {
    t = saturate(t);
    float scale = ease_back_out(t);
    float fly = ease_p3_out(t);
    auto& transform = e.get<Transform2D>();
    transform.setScale(scale);
    transform.setY(animationVertDistance * (1.0f - fly));
}

void on_popup_opened(EntityApi e) {
    setTouchable(e, true);
}

void on_popup_closing(EntityApi e) {
    setTouchable(e, false);
}

void on_popup_closed(EntityApi e) {
    EntityApi* it = g_popup_manager.active.find(e);
    if (it) {
        g_popup_manager.active.erase_ptr(it);
    }

    if (g_popup_manager.active.empty()) {
        ecs::EntityApi pm = g_popup_manager.entity;
        setTouchable(pm, false);
        setVisible(pm, false);
    } else {
        on_popup_resume(g_popup_manager.active.back());
    }

    setVisible(e, false);
    // TODO: flag auto-delete
    destroyDelay(e);
}

void on_popup_close_animation(float t, EntityApi e) {
    t = saturate(1 - t);
    float scale = ease_back_out(t);
    float fly = ease_p3_out(t);
    auto& transform = e.get<Transform2D>();
    transform.setScale(scale);
    transform.setY(animationVertDistance * (fly - 1.0f));
}

void init_basic_popup(EntityApi e) {
    auto node_close = find(e, H("btn_close"));
    if (node_close) {
        auto* btn = node_close.tryGet<Button>();
        if(btn) {
            btn->back_button = true;
            node_close.get_or_create<NodeEventHandler>().on(BUTTON_EVENT_CLICK, [e](const NodeEventData& event) {
                close_popup(e);
            });
        }
    }
}

void open_popup(EntityApi e) {
    g_popup_manager.closingLast = nullptr;

    // if we have entity in active list - do nothing
    if (g_popup_manager.active.find(e) != nullptr) {
        return;
    }

    if (e.get<Node>().parent == g_popup_manager.layer) {
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
    tween.advanced += [e](float r) {
        if (r >= 1.0f) {
            on_popup_opened(e);
        }
        on_popup_open_animation(r, e);
    };

    append(g_popup_manager.layer, e);
    auto& st = g_popup_manager.entity.get<Node>();
    st.setTouchable(true);
    st.setVisible(true);
}

void close_popup(EntityApi e) {
    // we cannot close entity if it is not active
    if (g_popup_manager.active.find(e) == nullptr) {
        return;
    }

    if (g_popup_manager.active.back() == e) {
        g_popup_manager.closingLast = e;
    }
    on_popup_closing(e);

    auto& tween = Tween::reset(e);
    tween.delay = tweenDelay;
    tween.duration = tweenDuration;
    tween.advanced += [e](float t) {
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

    g_popup_manager.closingLast = nullptr;
    g_popup_manager.fade_progress = 0.0f;
    setAlpha(g_popup_manager.back, 0.0f);

    destroyChildren(g_popup_manager.layer);
    g_popup_manager.active.clear();

    const ecs::EntityApi e = g_popup_manager.entity;
    setTouchable(e, false);
    setVisible(e, false);
}

void close_all_popups() {
    auto copy_vec = g_popup_manager.active;
    for (auto p: copy_vec) {
        close_popup(p);
    }
}

ecs::EntityApi createBackQuad() {
    auto e = createNode2D(H("back"));
    auto& display = e.assign<Display2D>();
    quad2d_setup(e.index)->setColor(COLOR_BLACK);
    display.program = R_SHADER_SOLID_COLOR;
    e.assign<LayoutRect>()
            .fill(true, true)
            .setInsetsMode(false);

    // intercept back-button if popup manager is active
    e.assign<Interactive>();
    auto& eh = e.assign<NodeEventHandler>();
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
    auto e = createNode2D(H("popups"));
    g_popup_manager.entity = e;
    auto& pm = g_popup_manager;
    pm.back = createBackQuad();
    append(e, pm.back);

    pm.layer = createNode2D(H("layer"));
    pm.layer.assign<LayoutRect>()
            .enableAlignX(0.5f)
            .enableAlignY(0.5f)
            .setInsetsMode(false);
    append(e, pm.layer);

    // initially popup manager is deactivated
    auto& st = e.get<Node>();
    st.setTouchable(false);
    st.setVisible(false);
}

void popup_manager_update() {
    using namespace ek;
    auto dt = g_time_layers[TIME_LAYER_UI].dt;
    auto& p = g_popup_manager;
    bool needFade = !p.active.empty();
    if (p.active.size() == 1 && p.active.back() == p.closingLast) {
        needFade = false;
    }
    p.fade_progress = reach(p.fade_progress,
                            needFade ? 1.0f : 0.0f,
                            dt / p.fade_duration);

    setAlpha(p.back, p.fade_alpha * p.fade_progress);

//    if (!p.active.empty()) {
//        auto front = p.active.back();
//        if (front.has<close_timeout>()) {
//            auto& t = front.get<close_timeout>();
//            t.time -= dt;
//            if (t.time <= 0.0f) {
//                front.remove<close_timeout>();
//                close_popup(front);
//            }
//        }
//    }
}

