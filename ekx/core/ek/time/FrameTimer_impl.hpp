#pragma once

#include "FrameTimer.hpp"

namespace ek {

float FrameTimer::update() {
    const float dt = static_cast<float>(timer_.readSeconds());
    deltaTime = dt;
    ++frameIndex;
    timer_.reset();
    return dt;
}

}