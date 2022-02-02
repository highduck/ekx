#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/ds/Array.hpp>
#include <ek/ds/String.hpp>
#include <ek/util/Signal.hpp>
#include <ek/math.h>

namespace ek {

#define GAME_SCREEN_EVENT_CREATE H("game-screen-create")
#define GAME_SCREEN_EVENT_DESTROY H("game-screen-destroy")
#define GAME_SCREEN_EVENT_ENTER H("game-screen-enter")
#define GAME_SCREEN_EVENT_ENTER_BEGIN H("game-screen-enter-begin")
#define GAME_SCREEN_EVENT_EXIT H("game-screen-exit")
#define GAME_SCREEN_EVENT_EXIT_BEGIN H("game-screen-exit-begin")

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

struct GameScreenManager {
    ecs::EntityApi layer;

    PodArray<ecs::EntityApi> stack;

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

void init_game_screen(ecs::EntityApi e, string_hash_t name = 0);

}

extern ek::GameScreenManager* g_game_screen_manager;
void init_game_screen_manager();
