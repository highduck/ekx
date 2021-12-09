#pragma once

#include "Timers.hpp"
#include "../ds/Hash.hpp"
#include <ek/time.h>

namespace ek {

struct TimerJob final {
    std::function<void()> fn;
    uint64_t time = 0;
    uint64_t repeat = 0;
};

uint64_t toNanoSeconds(double time) {
    return (uint64_t)(time * 1000000000.0);
}

class TimerJobManager final {
public:
    void update() {
        updateTimers();
        destroyEmptyTimers();
    }

    int create(std::function<void()> fn, uint64_t timeout, uint64_t interval) {
        ++nextID;
        TimerJob timer{};
        timer.fn = std::move(fn);
        timer.time = ek_ticks(nullptr) + timeout;
        timer.repeat = interval;
        jobs.set(nextID, std::move(timer));
        return nextID;
    }

    bool cancel(int id) {
        auto* timer = (TimerJob*) jobs.tryGet(id);
        if (timer) {
            timer->fn = nullptr;
            return true;
        }
        return false;
    }

    static TimerJobManager* instance;

private:
    void updateTimers() {
        auto now = ek_ticks(nullptr);
        for (uint32_t i = 0; i < jobs.size(); ++i) {
            auto& entry = jobs._data[i];
            if (now >= entry.value.time) {
                entry.value.fn();
                if (entry.value.repeat != 0) {
                    entry.value.time += entry.value.repeat;
                } else {
                    entry.value.fn = nullptr;
                }
            }
        }
    }

    void destroyEmptyTimers() {
        uint32_t i = 0;
        while (i < jobs.size()) {
            auto& entry = jobs._data[i];
            if (!entry.value.fn) {
                jobs.remove(entry.key);
            } else {
                ++i;
            }
        }
    }

    Hash<TimerJob> jobs;
    int nextID = 0;
};

inline TimerJobManager* TimerJobManager::instance = nullptr;

int setTimeout_(std::function<void()> fn, double timeout) {
    return TimerJobManager::instance->create(std::move(fn), toNanoSeconds(timeout), 0);
}

int setInterval_(std::function<void()> fn, double interval) {
    return TimerJobManager::instance->create(std::move(fn), toNanoSeconds(interval), toNanoSeconds(interval));
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

void setupTimers() {
    TimerJobManager::instance = new TimerJobManager();
}

}