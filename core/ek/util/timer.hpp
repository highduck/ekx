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

class fps_meter_t final {
public:
    void update(float dt) {
        // estimate average FPS for some period
        counter_ += 1.0f;
        accum_ += dt;
        if (accum_ >= avg_period_) {
            avg_fps_ = counter_ / avg_period_;
            accum_ -= avg_period_;
            counter_ = 0.0f;
        }

        // update immediate frame FPS count
        frame_fps_ = dt > (1.0f / max_frame_fps_) ? (1.0f / dt) : max_frame_fps_;
    }

    [[nodiscard]]
    float fps() const {
        return avg_fps_;
    }

    [[nodiscard]]
    float frame_fps() const {
        return frame_fps_;
    }

private:
    const float avg_period_ = 1.0f;
    const float max_frame_fps_ = 1000.0f;

    float avg_fps_ = 0.0f;
    float frame_fps_ = 0.0f;
    float counter_ = 0.0f;
    float accum_ = 0.0f;
};

}