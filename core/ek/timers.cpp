#include "timers.hpp"

#include <map>
#include <memory>
#include <ek/util/timer.hpp>
#include <utility>

namespace ek {

struct Timer final {
    std::function<void()> fn;
    double time = 0.0;
    double repeat = 0.0;
};

class Runner final {
public:
    void update() {
        updateTimers();
        destroyEmptyTimers();
    }

    int addTimer(std::function<void()> fn, double timeout, double interval) {
        ++nextID;
        auto timer = std::make_unique<Timer>();
        timer->fn = std::move(fn);
        timer->time = clock.read_seconds() + timeout;
        timer->repeat = interval;
        timers[nextID] = std::move(timer);
        return nextID;
    }

    static Runner& instance() {
        if (!instance_) {
            instance_ = new Runner{};
        }
        return *instance_;
    }

    bool cancel(int id) {
        auto it = timers.find(id);
        if (it != timers.end()) {
            auto& timer = it->second;
            if (timer) {
                timer.reset();
                return true;
            }
        }
        return false;
    }

private:
    void updateTimers() {
        auto now = clock.read_seconds();
        for (auto& p : timers) {
            auto& timer = p.second;
            if (timer) {
                if (now >= timer->time) {
                    timer->fn();
                    if (timer->repeat > 0.0f) {
                        timer->time += timer->repeat;
                    } else {
                        timer.reset();
                    }
                }
            }
        }
    }

    void destroyEmptyTimers() {
        auto it = timers.begin();
        while (it != timers.end()) {
            if (it->second) {
                it++;
            } else {
                it = timers.erase(it);
            }
        }
    }

    std::map<int, std::unique_ptr<Timer>> timers{};
    ek::timer_t clock{};
    int nextID = 0;

    static Runner* instance_;
};

Runner* Runner::instance_{};

int setTimeout(std::function<void()> fn, double timeout) {
    return Runner::instance().addTimer(std::move(fn), timeout, 0.0f);
}

int setInterval(std::function<void()> fn, double interval) {
    return Runner::instance().addTimer(std::move(fn), interval, interval);
}

bool cancelTimer(int id) {
    return Runner::instance().cancel(id);
}

bool clearInterval(int id) {
    return cancelTimer(id);
}

bool clearTimeout(int id) {
    return cancelTimer(id);
}

void dispatchTimers() {
    Runner::instance().update();
}

}