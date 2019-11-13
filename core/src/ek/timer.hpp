#pragma once

#include <cstdint>

namespace ek {

class clock_device_t final {
public:
    using counter_type = uint64_t;
    using frequency_type = uint64_t;

    clock_device_t() noexcept;

    ~clock_device_t() = default;

    [[nodiscard]]
    inline counter_type counter() const {
        return (*query_counter_func_)();
    }

    [[nodiscard]]
    inline frequency_type frequency() const {
        return frequency_;
    }

private:
    frequency_type frequency_;

    counter_type (* query_counter_func_)();
};

const clock_device_t& get_clock_device();

class timer_t final {
public:
    using ticks_type = typename clock_device_t::counter_type;
    using seconds_type = float;
    using millis_type = float;

    timer_t()
            : initial_ticks_{get_clock_device().counter()} {
    }

    ~timer_t() = default;

    [[nodiscard]]
    inline ticks_type read_absolute_ticks() const {
        return get_clock_device().counter();
    }

    [[nodiscard]]
    seconds_type read_seconds() const {
        return static_cast<seconds_type>(
                static_cast<double>(read_ticks()) / frequency()
        );
    }

    [[nodiscard]]
    millis_type read_millis() const {
        return static_cast<millis_type>(
                1000.0 * (static_cast<double>(read_ticks()) / frequency())
        );
    }

    [[nodiscard]]
    ticks_type read_ticks() const {
        return read_absolute_ticks() - initial_ticks();
    }

    [[nodiscard]]
    inline ticks_type initial_ticks() const {
        return initial_ticks_;
    }

    [[nodiscard]]
    inline ticks_type frequency() const {
        return get_clock_device().frequency();
    }

    inline void reset() {
        initial_ticks_ = get_clock_device().counter();
    }

private:
    ticks_type initial_ticks_;
};

class framed_timer_t {
public:
    float update() {
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
    inline float delta_time() const {
        return delta_time_;
    }

private:
    timer_t timer_{};
    float delta_time_ = 0.0f;
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