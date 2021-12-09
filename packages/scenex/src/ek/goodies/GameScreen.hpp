#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/ds/Array.hpp>
#include <ek/ds/String.hpp>
#include <ek/util/Signal.hpp>
#include <ek/util/Type.hpp>
#include <ek/math/Rect.hpp>

namespace ek {

class GameScreenHandler {
public:
    virtual ~GameScreenHandler() = default;

    virtual void onScreenCreate() = 0;

    virtual void onScreenEnter() = 0;
    virtual void onScreenEnterBegin() = 0;
    virtual void onScreenExit() = 0;
    virtual void onScreenExitBegin() = 0;

    // by default just destroy all children
    virtual void onScreenDestroy() = 0;
};

enum class GameScreenEvent {
    Create,
    Destroy,
    Enter,
    EnterBegin,
    Exit,
    ExitBegin
};

struct GameScreen {
    Signal<GameScreenEvent> onEvent;

    // do not create/destroy content
    //bool persistent = true;

    static GameScreen& init(ecs::EntityApi e, const char* name = nullptr);
};

ECX_TYPE(21, GameScreen);

struct ScreenTransitionState {
    bool active = false;
    float t = 0.0f;

    float prevTimeStart = 0.0f;
    float prevTimeEnd = 0.5f;
    float nextTimeStart = 0.5f;
    float nextTimeEnd = 1.0f;

    bool prevPlayStarted = false;
    bool prevPlayCompleted = false;
    bool nextPlayStarted = false;
    bool nextPlayCompleted = false;

    ecs::EntityApi prev;
    ecs::EntityApi next;

    float duration = 0.65f;
    float delay = 0.15f;
    float delayTimer = 0.0f;

    Rect2f screenRect{};

    void checkStates();

    void beginPrev();

    void completePrev();

    void beginNext();

    void completeNext();

    [[nodiscard]] float getPrevProgress() const;

    [[nodiscard]] float getNextProgress() const;

};

class GameScreenManager {
public:
    ecs::EntityApi layer;

    Array<ecs::EntityApi> stack;

    ScreenTransitionState transition;

    std::function<void(ScreenTransitionState&)> transitionEffect = defaultTransitionEffect;

    explicit GameScreenManager(ecs::EntityApi layer_);

    void setScreen(const char* name);

    [[nodiscard]]
    ecs::EntityApi findScreen(const char* name) const;

    void changeScreen(const char* name);

    void update();

    void applyTransitionEffect(ScreenTransitionState& state) const;

    static void defaultTransitionEffect(ScreenTransitionState& state);
};

EK_DECLARE_TYPE(GameScreenManager);

}

