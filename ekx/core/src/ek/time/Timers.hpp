#pragma once

#include <functional>

namespace ek {

int setTimeout(std::function<void()> callback, double timeout);

int setInterval(std::function<void()> callback, double interval);

bool cancelTimer(int id);

bool clearInterval(int id);

bool clearTimeout(int id);

void dispatchTimers();

void setupTimers();

}