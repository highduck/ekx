#include "GameScreen.hpp"


#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/log.h>
#include <ek/scenex/2d/LayoutRect.hpp>
#include <ek/scenex/base/TimeLayer.hpp>

namespace ek {

/** GameScreen component **/
GameScreen& GameScreen::init(ecs::EntityApi e, string_hash_t name) {
    auto& node = e.get<Node>();
    if(name) {
        node.tag = name;
    }
    node.setVisible(false);
    node.setTouchable(false);
    return e.reassign<GameScreen>();
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

void ScreenTransitionState::beginPrev() {
    if (prev) {
        //prev.get<GameScreen>().onExitBegin();
        prev.get<GameScreen>().onEvent.emit(GameScreenEvent::ExitBegin);
        //broadcast(screenPrev, GameScreen::ExitBegin);
    }
    prevPlayStarted = true;
}

void ScreenTransitionState::completePrev() {
    if (prev) {
        prev.get<Node>().setVisible(false);
        prev.get<Node>().setTouchable(false);
        //prev.get<GameScreen>().onExit();
        prev.get<GameScreen>().onEvent.emit(GameScreenEvent::Exit);
        //broadcast(prev, GameScreen::Exit);
    }
    prev = nullptr;
    prevPlayCompleted = true;
}

void ScreenTransitionState::beginNext() {
    nextPlayStarted = true;
    if (next) {
        next.get<Node>().setVisible(true);
        next.get<Node>().setTouchable(true);
        //next.get<GameScreen>().onEnterBegin();
        next.get<GameScreen>().onEvent.emit(GameScreenEvent::EnterBegin);
        //broadcast(screenNext, GameScreen::EnterBegin);
    }
}

void ScreenTransitionState::completeNext() {
    if (next) {
        next.get<GameScreen>().onEvent.emit(GameScreenEvent::Enter);
        //next.get<GameScreen>().onEnter();
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

GameScreenManager::GameScreenManager(ecs::EntityApi layer_) :
        layer{layer_} {
}

void GameScreenManager::setScreen(string_hash_t name) {
    if (transition.active) {
        return;
    }

    // hide all activated screens
    for (auto it: stack) {
        auto& node = it.get<Node>();
        node.setVisible(false);
        node.setTouchable(false);
    }

    stack.clear();

    auto e = findScreen(name);
    if (e) {
        stack.push_back(e);

        auto& node = e.get<Node>();
        node.setVisible(true);
        node.setTouchable(true);

        // bring to the top
        removeFromParent(e);
        append(layer, e);

        // set fully shown transform
        transition = {};
        transition.next = e;
        transition.t = 1.0f;
        applyTransitionEffect();

        //e.get<GameScreen>().onEnterBegin();
        e.get<GameScreen>().onEvent.emit(GameScreenEvent::EnterBegin);

        //broadcast(layer, GameScreen::EnterBegin);

        // TODO:
        //analytics::screen(name.c_str());
    }
}

ecs::EntityApi GameScreenManager::findScreen(string_hash_t name) const {
    auto e = find(layer, name);
    if(e && e.has<GameScreen>()) {
        return e;
    }
    EK_DEBUG("could not find screen %s (%08X)", hsp_get(name), name);
    return 0;
}

void GameScreenManager::changeScreen(string_hash_t name) {
    if (transition.active) {
        return;
    }
    transition = {};
    transition.active = true;
    transition.delayTimer = transition.delay;
    transition.screenRect = find_parent_layout_rect(layer, false);
    setTouchable(layer, false);

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
        removeFromParent(transition.next);
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
    const auto dt = TimeLayer::UI->dt;
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
            setTouchable(layer, true);
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
        auto& transform = prev.get<Transform2D>();
        float r = ease_p2_in(t);

        transform.color.scale.a = unorm8_f32_clamped(1.0f - r);
        //transform.color.setAdditive(r * r);
        float s = 1.0f + r * 0.3f;
        transform.setScale(s);
        transform.setPosition({}, {}, rect_center(state.screenRect));
    }

    if (next) {
        const auto t = state.getNextProgress();
        auto& transform = next.get<Transform2D>();
        float r = ease_p2_out(t);
        transform.color.scale.a = unorm8_f32_clamped(r);
        //transform.color.offset.a = unorm8_f32_clamped(((1.0f - r) * (1.0f - r)));
        float s = 1.0f + (1.0f - r) * 0.3f;
        transform.setScale(s);
        transform.setPosition({}, {}, rect_center(state.screenRect));
    }
}
}