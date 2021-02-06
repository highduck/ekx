#include "timers.hpp"

#define SOKOL_TIME_IMPL

#include <sokol_time.h>

#include <map>
//#include <memory>
//#include <utility>

namespace ek {

struct TimerJob final {
    std::function<void()> fn;
    double time = 0.0;
    double repeat = 0.0;
};

class TimerJobManager final {
public:
    void update() {
        updateTimers();
        destroyEmptyTimers();
    }

    int create(std::function<void()> fn, double timeout, double interval) {
        ++nextID;
        auto timer = std::make_unique<TimerJob>();
        timer->fn = std::move(fn);
        timer->time = clock.read_seconds() + timeout;
        timer->repeat = interval;
        jobs[nextID] = std::move(timer);
        return nextID;
    }

    bool cancel(int id) {
        auto it = jobs.find(id);
        if (it != jobs.end()) {
            auto& timer = (*it).second;
            if (timer) {
                timer.reset();
                return true;
            }
        }
        return false;
    }

    static TimerJobManager* instance;

private:
    void updateTimers() {
        auto now = clock.read_seconds();
        for (auto& p : jobs) {
            auto& job = p.second;
            if (job) {
                if (now >= job->time) {
                    job->fn();
                    if (job->repeat > 0.0f) {
                        job->time += job->repeat;
                    } else {
                        job.reset();
                    }
                }
            }
        }
    }

    void destroyEmptyTimers() {
        auto it = jobs.begin();
        while (it != jobs.end()) {
            if (it->second) {
                it++;
            } else {
                it = jobs.erase(it);
            }
        }
    }

    std::map<int, std::unique_ptr<TimerJob>> jobs{};
    ek::timer_t clock{};
    int nextID = 0;
};

TimerJobManager* TimerJobManager::instance = nullptr;

int setTimeout(std::function<void()> fn, double timeout) {
    return TimerJobManager::instance->create(std::move(fn), timeout, 0.0f);
}

int setInterval(std::function<void()> fn, double interval) {
    return TimerJobManager::instance->create(std::move(fn), interval, interval);
}

bool cancelTimer(int id) {
    return TimerJobManager::instance->cancel(id);
}

bool clearInterval(int id) {
    return cancelTimer(id);
}

bool clearTimeout(int id) {
    return cancelTimer(id);
}

void dispatchTimers() {
    TimerJobManager::instance->update();
}

/** Time Layer system **/

float updateState(TimeLayer::State& layer, float dt_) {
    auto dt1 = dt_ * layer.scale;
    layer.dt = dt1;
    layer.total += dt1;
    return dt1;
}

const auto MAX_DELTA_TIME = 0.3;

void TimeLayer::updateTimers(float raw) {
    if (raw > MAX_DELTA_TIME) {
        raw = MAX_DELTA_TIME;
    }
    auto dt = updateState(TimeLayer::Layers[0], raw);
    updateState(TimeLayer::Layers[1], dt);
    updateState(TimeLayer::Layers[2], dt);
    updateState(TimeLayer::Layers[3], dt);
}

TimeLayer::State TimeLayer::Layers[4]{};

TimeLayer TimeLayer::Root{0};
TimeLayer TimeLayer::Game{1};
TimeLayer TimeLayer::HUD{2};
TimeLayer TimeLayer::UI{3};

namespace clock {

void init() {
    stm_setup();
    TimerJobManager::instance = new TimerJobManager();
}

double now() {
    return stm_sec(stm_now());
}

}

timer_t::timer_t() : initial_{stm_now()} {

}

double timer_t::read_seconds() const {
    return stm_sec(stm_diff(stm_now(), initial_));
}

double timer_t::read_millis() const {
    return stm_ms(stm_diff(stm_now(), initial_));
}

void timer_t::reset() {
    initial_ = stm_now();
}

double framed_timer_t::update() {
    ++frame_index_;
    delta_time_ = timer_.read_seconds();
    timer_.reset();
    return delta_time_;
}

void FpsMeter::update(float dt) {
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
}