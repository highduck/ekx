#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/ds/Array.hpp>
#include <ek/ds/String.hpp>
#include <ek/util/Signal.hpp>
#include <ek/math.h>

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

    static GameScreen& init(ecs::EntityApi e, string_hash_t name = 0);
};



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

    rect_t screenRect = {};

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

    void(*transitionEffect)(GameScreenManager* gsm) = defaultTransitionEffect;

    GameScreenManager();

    void setScreen(string_hash_t name);

    [[nodiscard]]
    ecs::EntityApi findScreen(string_hash_t name) const;

    void changeScreen(string_hash_t name);

    void update();

    void applyTransitionEffect();

    static void defaultTransitionEffect(GameScreenManager* gsm);
};

}

extern ek::GameScreenManager* g_game_screen_manager;
void init_game_screen_manager();
