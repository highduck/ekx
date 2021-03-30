#pragma once

#include <ecxx/ecxx.hpp>
#include <vector>
#include <string>
#include <ek/util/signals.hpp>
#include <ek/math/box.hpp>

namespace ek {

struct GameScreen {
    //inline static const std::string EnterBegin = "game_screen_enter_begin";
    //inline static const std::string Enter = "game_screen_enter";
    //inline static const std::string ExitBegin = "game_screen_exit_begin";
    //inline static const std::string Exit = "game_screen_exit";

    signal_t<> onEnter;
    signal_t<> onEnterBegin;
    signal_t<> onExit;
    signal_t<> onExitBegin;

    std::function<void()> fnCreate;

    // by default just destroy all children
    std::function<void()> fnDestroy;

    // do not create/destroy content
    bool persistent = true;

    static GameScreen& init(ecs::EntityApi e, const char* name = nullptr);
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

    rect_f screenRect{};

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

    std::vector<ecs::EntityApi> stack;

    ScreenTransitionState transition;

    std::function<void(ScreenTransitionState&)> transitionEffect = defaultTransitionEffect;

    explicit GameScreenManager(ecs::EntityApi layer_);

    void setScreen(const std::string& name);

    [[nodiscard]]
    ecs::EntityApi findScreen(const std::string& name) const;

    void changeScreen(const std::string& name);

    void update();

    void applyTransitionEffect(ScreenTransitionState& state) const;

    static void defaultTransitionEffect(ScreenTransitionState& state);
};

}

