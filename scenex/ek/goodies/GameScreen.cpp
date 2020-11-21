#include "GameScreen.hpp"

namespace ek {

/** GameScreen component **/
GameScreen& GameScreen::init(ecs::entity e, const char* name) {
    if (name) {
        e.get<Node>().name = name;
    }
    e.get<Node>().setVisible(false);
    e.get<Node>().setTouchable(false);
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
        prev.get<Transform2D>().origin = screenRect.center();
        prev.get<GameScreen>().onExitBegin();
        //broadcast(screenPrev, GameScreen::ExitBegin);
    }
    prevPlayStarted = true;
}

void ScreenTransitionState::completePrev() {
    if (prev) {
        prev.get<Node>().setVisible(false);
        prev.get<Node>().setTouchable(false);
        prev.get<GameScreen>().onExit();
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
        next.get<Transform2D>().origin = screenRect.center();
        next.get<GameScreen>().onEnterBegin();
        //broadcast(screenNext, GameScreen::EnterBegin);
    }
}

void ScreenTransitionState::completeNext() {
    if (next) {
        next.get<GameScreen>().onEnter();
        //broadcast(next, GameScreen::Enter);
    }
    next = nullptr;
    nextPlayCompleted = true;
}

float ScreenTransitionState::getPrevProgress() const {
    return math::clamp((t - prevTimeStart) / (prevTimeEnd - prevTimeStart));
}

float ScreenTransitionState::getNextProgress() const {
    return math::clamp((t - nextTimeStart) / (nextTimeEnd - nextTimeStart));
}

/** GameScreenManager **/



GameScreenManager::GameScreenManager(ecs::entity layer_) :
        layer{layer_} {
}

void GameScreenManager::setScreen(const std::string& name) {
    if (transition.active) {
        return;
    }

    // hide all activated screens
    for (auto it : stack) {
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
        applyTransitionEffect(transition);

        e.get<GameScreen>().onEnterBegin();
        //broadcast(layer, GameScreen::EnterBegin);

        analytics::screen(name.c_str());
    }
}

ecs::entity GameScreenManager::findScreen(const std::string& name) const {
    auto it = layer.get<Node>().child_first;
    while (it) {
        if (it.get<Node>().name == name && it.has<GameScreen>()) {
            return it;
        }
        it = it.get<Node>().sibling_next;
    }
    EK_DEBUG << "could not find screen: " << name;
    return nullptr;
}

void GameScreenManager::changeScreen(const std::string& name) {
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
    applyTransitionEffect(transition);

    analytics::screen(name.c_str());
}

void GameScreenManager::update() {
    if (!transition.active) {
        return;
    }
    const auto dt = TimeLayer::UI->dt;
    if (transition.delayTimer > 0.0f) {
        transition.delayTimer -= dt;
    } else if (transition.t < 1.0f) {
        transition.t += dt * (1.0f / transition.duration);
        if (transition.t > 1.0f) {
            transition.t = 1.0f;
        }

        transition.checkStates();
        applyTransitionEffect(transition);

        if (transition.nextPlayCompleted && transition.prevPlayCompleted) {
            transition.active = false;
            setTouchable(layer, true);
        }
    }
}

void GameScreenManager::applyTransitionEffect(ScreenTransitionState& state) const {
    if (transitionEffect) {
        transitionEffect(state);
    }
//        else {
//            defaultTransitionEffect(prev, next, progress)
//        }
}

void GameScreenManager::defaultTransitionEffect(ScreenTransitionState& state) {
    const auto next = state.next;
    const auto prev = state.prev;

    if (prev) {
        const auto t = state.getPrevProgress();
        auto& transform = prev.get<Transform2D>();
        float r = easing::P2_IN.calculate(t);
        transform.color.setAlpha(1.0f - r);
        //transform.color.setAdditive(r);
        float s = 1.0f + r * 0.3f;
        transform.scale = {s, s};
    }

    if (next) {
        const auto t = state.getNextProgress();
        auto& transform = next.get<Transform2D>();
        float r = easing::P2_OUT.calculate(t);
        transform.color.setAlpha(r);
        //transform.color.setAdditive(1.0f - r);
        float s = 1.0f + (1.0f - r) * 0.3f;
        transform.scale = {s, s};
    }
}
}