#pragma once

#include <functional>

namespace ek {

namespace clock {
void initialize();

double now();
}

/** Timeout Jobs **/

int setTimeout(std::function<void()> callback, double timeout);

int setInterval(std::function<void()> callback, double interval);

bool cancelTimer(int id);

bool clearInterval(int id);

bool clearTimeout(int id);

void dispatchTimers();

/** timer classes **/
class timer_t final {
public:
    timer_t();

    [[nodiscard]]
    double read_seconds() const;

    [[nodiscard]]
    double read_millis() const;

    void reset();

private:
    uint64_t initial_;
};

class framed_timer_t {
public:
    double update();

    [[nodiscard]]
    inline uint64_t frame_index() const {
        return frame_index_;
    }

    [[nodiscard]]
    inline double delta_time() const {
        return delta_time_;
    }

private:
    timer_t timer_{};
    double delta_time_ = 0.0;
    uint64_t frame_index_ = 0ull;
};

class FpsMeter final {
public:
    void update(float dt);

    [[nodiscard]]
    float getAverageFPS() const {
        return avgFPS_;
    }

private:
    // if less or equal 0 - calculate FPS for every frame
    const float measurementsPerSeconds = 2.0f;
    float avgFPS_ = 0.0f;
    float counter_ = 0.0f;
    float accum_ = 0.0f;
};

/** Time Layer System **/

struct TimeLayer {
    struct State {
        float scale = 1.0f;
        float dt = 0.0f;
        float total = 0.0f;
        float pad_ = 0.0f;
    };

    static State Layers[4];

    TimeLayer() = default;

    explicit TimeLayer(unsigned char id_) :
            id{id_} {

    }

    inline const State* operator->() const {
        return Layers + id;
    }

    inline State* operator->() {
        return Layers + id;
    }

    static TimeLayer Root;
    static TimeLayer Game;
    static TimeLayer HUD;
    static TimeLayer UI;

    static void updateTimers(float raw);

private:
    unsigned char id = 0;
};

}

