#pragma once

#include <functional>

namespace ek {

int setTimeout_(std::function<void()> callback, double timeout);

int setInterval_(std::function<void()> callback, double interval);

template<typename Fn>
int setTimeout(Fn&& callback, double timeout) {
    return setTimeout_(callback, timeout);
}

template<typename Fn>
int setInterval(Fn&& callback, double interval) {
    return setInterval_(callback, interval);
}

bool cancelTimer(int id);

bool clearInterval(int id);

bool clearTimeout(int id);

void dispatchTimers();

void setupTimers();

}