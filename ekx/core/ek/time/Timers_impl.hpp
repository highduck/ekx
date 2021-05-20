#pragma once

#include "Timers.hpp"
#include <map>
#include <memory>
#include "Stopwatch.hpp"

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
        timer->time = clock.readSeconds() + timeout;
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
        auto now = clock.readSeconds();
        for (auto& p : jobs) {
            auto& job = p.second;
            if (job && now >= job->time) {
                job->fn();
                if(job) {
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
    ek::Stopwatch clock{};
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

void setupTimers() {
    TimerJobManager::instance = new TimerJobManager();
}

}