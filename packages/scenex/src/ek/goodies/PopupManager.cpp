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

EntityApi PopupManager::Main{};

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
    auto pm = PopupManager::Main;
    auto& state = pm.get<PopupManager>();
    EntityApi* it = state.active.find(e);
    if (it) {
        state.active.eraseIterator(it);
    }

    if (state.active.empty()) {
        setTouchable(pm, false);
        setVisible(pm, false);
    } else {
        on_popup_resume(state.active.back());
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
    if (node_close && node_close.has<Button>()) {
        auto& btn_close = node_close.get<Button>();
        btn_close.clicked += [e] {
            close_popup(e);
        };
        btn_close.back_button = true;
    }
}

void PopupManager::updateAll() {
    auto dt = g_time_layers[TIME_LAYER_UI].dt;
    for (auto e: ecs::view<PopupManager>()) {
        auto& p = e.get<PopupManager>();
        bool needFade = !p.active.empty();
        if (p.active.size() == 1 && p.active.back() == p.closingLast) {
            needFade = false;
        }
        p.fade_progress = reach(p.fade_progress,
                                      needFade ? 1.0f : 0.0f,
                                      dt / p.fade_duration);

        setAlpha(p.back, p.fade_alpha * p.fade_progress);

        if (!p.active.empty()) {
            auto front = p.active.back();
            if (front.has<close_timeout>()) {
                auto& t = front.get<close_timeout>();
                t.time -= dt;
                if (t.time <= 0.0f) {
                    front.remove<close_timeout>();
                    close_popup(front);
                }
            }
        }
    }
}

void open_popup(EntityApi e) {
    auto pm = PopupManager::Main;
    auto& state = pm.get<PopupManager>();

    state.closingLast = nullptr;

    // if we have entity in active list - do nothing
    if (state.active.find(e) != nullptr) {
        return;
    }

    if (e.get<Node>().parent == state.layer) {
        return;
    }

    if (!state.active.empty()) {
        on_popup_pause(state.active.back());
    }
    state.active.push_back(e);
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

    append(state.layer, e);
    auto& st = pm.get_or_create<Node>();
    st.setTouchable(true);
    st.setVisible(true);
}

void close_popup(EntityApi e) {
    auto pm = PopupManager::Main;
    auto& state = pm.get<PopupManager>();

    // we cannot close entity if it is not active
    if (state.active.find(e) == nullptr) {
        return;
    }

    if (state.active.back() == e) {
        state.closingLast = e;
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
    auto pm = PopupManager::Main;
    auto& state = pm.get<PopupManager>();
    return state.active.size();
}

void clear_popups() {
    auto pm = PopupManager::Main;
    auto& state = pm.get<PopupManager>();

    state.closingLast = nullptr;
    state.fade_progress = 0.0f;
    setAlpha(state.back, 0.0f);

    destroyChildren(state.layer);
    state.active.clear();
    setTouchable(pm, false);
    setVisible(pm, false);
}

void close_all_popups() {
    auto pm = PopupManager::Main;
    auto& state = pm.get<PopupManager>();

    auto copy_vec = state.active;
    for (auto p: copy_vec) {
        close_popup(p);
    }
}

ecs::EntityApi createBackQuad() {
    auto e = createNode2D(H("back"));
    auto& display = e.assign<Display2D>();
    display.makeDrawable<Quad2D>().setColor(COLOR_BLACK);
    display.program = R_SHADER_SOLID_COLOR;
    e.assign<LayoutRect>()
            .fill(true, true)
            .setInsetsMode(false);

    // intercept back-button if popup manager is active
    auto& eh = e.assign<NodeEventHandler>();
    eh.on(interactive_event::back_button, [](const NodeEventData& ev) {
        ev.processed = true;
    });

    // if touch outside of popups, simulate back-button behavior
    auto& interactive = e.assign<Interactive>();
    interactive.onEvent += [e](auto event) {
        if (event == PointerEvent::Down) {
            const auto* state = findComponentInParent<PopupManager>(e);
            if (state && !state->active.empty()) {
                g_interaction_system->sendBackButton();
            }
        }
    };

    return e;
}

ecs::EntityApi PopupManager::make() {
    auto e = createNode2D(H("popups"));
    auto& pm = e.assign<PopupManager>();
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

    // safe first default popup-manager
    if (!Main) {
        Main = e;
    }
    return e;
}
}