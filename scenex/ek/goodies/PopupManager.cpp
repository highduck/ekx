#include "PopupManager.hpp"

#include <ek/scenex/utility/destroy_delay.hpp>
#include <ek/scenex/utility/scene_management.hpp>
#include <ek/scenex/base/Interactive.hpp>
#include <ek/scenex/InteractionSystem.hpp>
#include <ek/scenex/2d/Button.hpp>
#include <ek/scenex/2d/LayoutRect.hpp>
#include <ek/math/easing.hpp>
#include <ek/util/locator.hpp>
#include <ek/scenex/base/Tween.hpp>

namespace ek {

using namespace ecs;

const float tweenDelay = 0.05f;
const float tweenDuration = 0.3f;
const float animationVertDistance = 200.0f;

static entity popups_;

void on_popup_pause(entity e) {
    setTouchable(e, false);
}

void on_popup_resume(entity e) {
    setTouchable(e, true);
}

void on_popup_opening(entity e) {
    e.get<Transform2D>().scale = float2::zero;
    e.get<Node>().setTouchable(false);
    e.get<Node>().setVisible(true);
}

void on_popup_open_animation(float t, entity e) {
    t = math::clamp(t, 0.0f, 1.0f);
    float scale = easing::BACK_OUT.calculate(t);
    float fly = easing::P3_OUT.calculate(t);
    auto& transform = e.get<Transform2D>();
    transform.position.y = animationVertDistance * (1.0f - fly);
    transform.scale = {scale, scale};
}

void on_popup_opened(entity e) {
    setTouchable(e, true);
}

void on_popup_closing(entity e) {
    setTouchable(e, false);
}

void on_popup_closed(entity e) {
    auto& state = ecs::get<PopupManager>(popups_);

    auto it = std::find(state.active.begin(), state.active.end(), e);
    if (it != state.active.end()) {
        state.active.erase(it);
    }

    if (state.active.empty()) {
        setTouchable(popups_, false);
        setVisible(popups_, false);
    } else {
        on_popup_resume(state.active.back());
    }

    setVisible(e, false);
    // TODO: flag auto-delete
    destroy_delay(e);
}

void on_popup_close_animation(float t, entity e) {
    t = math::clamp(1.0f - t, 0.0f, 1.0f);
    float scale = easing::BACK_OUT.calculate(t);
    float fly = easing::P3_OUT.calculate(t);
    auto& transform = ecs::get<Transform2D>(e);
    transform.position.y = animationVertDistance * (fly - 1.0f);
    transform.scale = {scale, scale};
}

void init_basic_popup(entity e) {
    auto node_close = find(e, "btn_close");
    if (node_close && ecs::has<Button>(node_close)) {
        auto& btn_close = ecs::get<Button>(node_close);
        btn_close.clicked.add([e]() {
            close_popup(e);
        });
        btn_close.back_button = true;
    }
}

bool contains(const std::vector<entity>& list, const entity e) {
    const auto it = std::find(list.begin(), list.end(), e);
    return it != list.end();
}

void PopupManager::updateAll() {
    auto dt = TimeLayer::UI->dt;
    for (auto e : ecs::view<PopupManager>()) {
        auto& p = ecs::get<PopupManager>(e);
        p.fade_progress = math::reach(p.fade_progress,
                                      p.active.empty() ? 0.0f : 1.0f,
                                      dt / p.fade_duration);

        setAlpha(p.back, p.fade_alpha * p.fade_progress);

        if (!p.active.empty()) {
            auto front = p.active.back();
            if (ecs::has<close_timeout>(front)) {
                auto& t = ecs::get<close_timeout>(front);
                t.time -= dt;
                if (t.time <= 0.0f) {
                    ecs::remove<close_timeout>(front);
                    close_popup(front);
                }
            }
        }
    }
}

void open_popup(entity e) {
    auto& state = popups_.get<PopupManager>();

    if (contains(state.active, e)) {
        return;
    }

    if (ecs::get<Node>(e).parent == state.layer) {
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
    auto& st = ecs::get_or_create<Node>(popups_);
    st.setTouchable(true);
    st.setVisible(true);
}

void close_popup(entity e) {
    auto& state = ecs::get<PopupManager>(popups_);
    if (!contains(state.active, e)) {
        return;
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
    auto count = popups_.get<PopupManager>().active.size();
    return static_cast<uint32_t>(count);
}

entity get_popup_manager() {
    return popups_;
}

void clear_popups() {
    auto& state = ecs::get<PopupManager>(popups_);

    state.fade_progress = 0.0f;
    setAlpha(state.back, 0.0f);

    destroyChildren(state.layer);
    state.active.clear();
    setTouchable(popups_, false);
    setVisible(popups_, false);
}

void close_all_popups() {
    auto& state = ecs::get<PopupManager>(popups_);
    auto copy_vec = state.active;
    for (auto p : copy_vec) {
        close_popup(p);
    }
}

ecs::entity createBackQuad() {
    auto e = create_node_2d("back");
    Display2D::make<Quad2D>(e).setColor(argb32_t::black);
    e.assign<LayoutRect>()
            .fill(true, true)
            .setInsetsMode(false);

    // intercept back-button if popup manager is active
    auto& eh = e.assign<event_handler_t>();
    eh.on(interactive_event::back_button, [](const event_data& ev) {
        ev.processed = true;
    });

    // if touch outside of popups, simulate back-button behavior
    auto& interactive = e.assign<interactive_t>();
    interactive.on_down += [e] {
        const auto* state = findComponentInParent<PopupManager>(e);
        if (state && !state->active.empty()) {
            resolve<InteractionSystem>().sendBackButton();
        }
    };

    return e;
}

ecs::entity PopupManager::make() {
    auto e = create_node_2d("popups");
    auto& pm = e.assign<PopupManager>();
    pm.back = createBackQuad();
    append(e, pm.back);

    pm.layer = create_node_2d("layer");
    pm.layer.assign<LayoutRect>()
            .enableAlignX(0.5f)
            .enableAlignY(0.5f)
            .setInsetsMode(false);
    append(e, pm.layer);

    // initially popup manager is deactivated
    auto& st = e.get_or_create<Node>();
    st.setTouchable(false);
    st.setVisible(false);

    popups_ = e;
    return e;
}
}