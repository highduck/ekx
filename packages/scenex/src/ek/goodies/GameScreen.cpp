#include "GameScreen.hpp"
#include "ek/scenex/base/NodeEvents.hpp"


#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/log.h>
#include <ek/scenex/2d/LayoutRect.hpp>
#include <ekx/app/time_layers.h>

namespace ek {

/** GameScreen component **/
void init_game_screen(ecs::Entity e, string_hash_t name) {
    if(name) {
        set_tag(e, name);
    }
    set_visible(e, false);
    set_touchable(e, false);
}

/** Transition **/

void ScreenTransitionState::checkStates() {
    if (!prevPlayStarted && t >= prevTimeStart) {
        prevPlayStarted = true;
        beginPrev();
    }
    if (!prevPlayCompleted && t >= prevTimeEnd) {
        prevPlayCompleted = true;
        completePrev();
    }
    if (!nextPlayStarted && t >= nextTimeStart) {
        nextPlayStarted = true;
        beginNext();
    }
    if (!nextPlayCompleted && t >= nextTimeEnd) {
        nextPlayCompleted = true;
        completeNext();
    }
}

void emit_screen_event(entity_t e, string_hash_t type) {
    auto* eh = ecs::try_get<NodeEventHandler>(e);
    if(eh) {
        eh->emit({type, e, {nullptr}, e});
    }
}

void ScreenTransitionState::beginPrev() {
    if (prev) {
        //ecs::get<GameScreen>(prev).onExitBegin();
        emit_screen_event(prev, GAME_SCREEN_EVENT_EXIT_BEGIN);
        //broadcast(screenPrev, GameScreen::ExitBegin);
    }
    prevPlayStarted = true;
}

void ScreenTransitionState::completePrev() {
    if (prev) {
        set_visible(prev, false);
        set_touchable(prev, false);
        //ecs::get<GameScreen>(prev).onExit();
        emit_screen_event(prev, GAME_SCREEN_EVENT_EXIT);
        //broadcast(prev, GameScreen::Exit);
    }
    prev = nullptr;
    prevPlayCompleted = true;
}

void ScreenTransitionState::beginNext() {
    nextPlayStarted = true;
    if (next) {
        set_visible(next, true);
        set_touchable(next, true);
        //ecs::get<GameScreen>(next).onEnterBegin();
        emit_screen_event(next, GAME_SCREEN_EVENT_ENTER_BEGIN);
        //ecs::get<GameScreen>(next).onEvent.emit(GameScreenEvent::EnterBegin);
        //broadcast(screenNext, GameScreen::EnterBegin);
    }
}

void ScreenTransitionState::completeNext() {
    if (next) {
        emit_screen_event(next, GAME_SCREEN_EVENT_ENTER);
        //ecs::get<GameScreen>(next).onEvent.emit(GameScreenEvent::Enter);
        //ecs::get<GameScreen>(next).onEnter();
        //broadcast(next, GameScreen::Enter);
    }
    next = nullptr;
    nextPlayCompleted = true;
}

float ScreenTransitionState::getPrevProgress() const {
    return saturate((t - prevTimeStart) / (prevTimeEnd - prevTimeStart));
}

float ScreenTransitionState::getNextProgress() const {
    return saturate((t - nextTimeStart) / (nextTimeEnd - nextTimeStart));
}

/** GameScreenManager **/

GameScreenManager::GameScreenManager() = default;

void GameScreenManager::setScreen(string_hash_t name) {
    if (transition.active) {
        return;
    }

    // hide all activated screens
    for (auto it: stack) {
        set_visible(it, false);
        set_touchable(it, false);
    }

    stack.clear();

    auto e = findScreen(name);
    if (e) {
        stack.push_back(e);

        set_visible(e, true);
        set_touchable(e, true);

        // bring to the top
        remove_from_parent(e);
        append(layer, e);

        // set fully shown transform
        transition = {};
        transition.next = e;
        transition.t = 1.0f;
        applyTransitionEffect();

        //ecs::get<GameScreen>(e).onEnterBegin();
        //ecs::get<GameScreen>(e).onEvent.emit(GameScreenEvent::EnterBegin);
        emit_screen_event(e, GAME_SCREEN_EVENT_ENTER_BEGIN);

        //broadcast(layer, GameScreen::EnterBegin);

        // TODO:
        //analytics::screen(name.c_str());
    }
}

ecs::Entity GameScreenManager::findScreen(string_hash_t name) const {
    auto e = find(layer, name);
    if(!e.id) {
        log_debug("could not find screen %s (%08X)", hsp_get(name), name);
    }
    return e;
}

void GameScreenManager::changeScreen(string_hash_t name) {
    if (transition.active) {
        return;
    }
    transition = {};
    transition.active = true;
    transition.delayTimer = transition.delay;
    transition.screenRect = find_parent_layout_rect(layer, false);
    set_touchable(layer, false);

    if (!stack.empty()) {
        transition.prev = stack.back();
        stack.pop_back();
    } else {
        transition.t = transition.nextTimeStart;
        transition.delayTimer = 0.0f;
    }

    transition.next = findScreen(name);
    if (transition.next) {
        stack.push_back(transition.next);

        // bring to the top
        remove_from_parent(transition.next);
        append(layer, transition.next);
    }

    transition.checkStates();
    applyTransitionEffect();

    // TODO:
    //analytics::screen(name.c_str());
}

void GameScreenManager::update() {
    if (!transition.active) {
        return;
    }
    const auto dt = g_time_layers[TIME_LAYER_UI].dt;
    if (transition.delayTimer > 0.0f) {
        transition.delayTimer -= dt;
    } else if (transition.t <= 1.0f) {
        transition.t += dt * (1.0f / transition.duration);
        if (transition.t > 1.0f) {
            transition.t = 1.0f;
        }

        applyTransitionEffect();
        transition.checkStates();

        if (transition.nextPlayCompleted && transition.prevPlayCompleted) {
            transition.active = false;
            set_touchable(layer, true);
        }
    }
}

void GameScreenManager::applyTransitionEffect() {
    if (transitionEffect) {
        transitionEffect(this);
    }
//        else {
//            defaultTransitionEffect(prev, next, progress)
//        }
}

void GameScreenManager::defaultTransitionEffect(GameScreenManager* gsm) {
    auto& state = gsm->transition;
    const auto next = state.next;
    const auto prev = state.prev;

    if (prev) {
        const auto t = state.getPrevProgress();
        auto& transform = ecs::get<Transform2D>(prev);
        float r = ease_p2_in(t);

        transform.color.scale.a = unorm8_f32_clamped(1.0f - r);
        //transform.color.setAdditive(r * r);
        float s = 1.0f + r * 0.3f;
        transform.set_scale(s);
        transform.set_position({}, {}, rect_center(state.screenRect));
    }

    if (next) {
        const auto t = state.getNextProgress();
        auto& transform = ecs::get<Transform2D>(next);
        float r = ease_p2_out(t);
        transform.color.scale.a = unorm8_f32_clamped(r);
        //transform.color.offset.a = unorm8_f32_clamped(((1.0f - r) * (1.0f - r)));
        float s = 1.0f + (1.0f - r) * 0.3f;
        transform.set_scale(s);
        transform.set_position({}, {}, rect_center(state.screenRect));
    }
}
}

ek::GameScreenManager* g_game_screen_manager = nullptr;
void init_game_screen_manager(void) {
    EK_ASSERT(!g_game_screen_manager);
    g_game_screen_manager = new ek::GameScreenManager();
}
