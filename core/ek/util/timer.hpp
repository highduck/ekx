#pragma once

#include <ek/util/clock.hpp>
#include <cstdint>

namespace ek {

class timer_t final {
    inline static double now() {
        return clock::now();
    }

public:
    [[nodiscard]]
    double read_seconds() const {
        return now() - initial_;
    }

    [[nodiscard]]
    double read_millis() const {
        return 1000.0 * read_seconds();
    }

    inline void reset() {
        initial_ = now();
    }

private:
    double initial_ = now();
};

class framed_timer_t {
public:
    double update() {
        ++frame_index_;
        delta_time_ = timer_.read_seconds();
        timer_.reset();
        return delta_time_;
    }

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
    void update(float dt) {
        if (measurementsPerSeconds > 0.0f) {
            // estimate average FPS for some period
            counter_ += 1.0f;
            accum_ += dt;
            const auto period = 1.0f / measurementsPerSeconds;
            if (accum_ >= period) {
                avgFPS_ = counter_ * measurementsPerSeconds;
                accum_ -= period;
                counter_ = 0.0f;
            }
        } else {
            avgFPS_ = dt > 0.0f ? (1.0f / dt) : 0.0f;
        }
    }

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

}