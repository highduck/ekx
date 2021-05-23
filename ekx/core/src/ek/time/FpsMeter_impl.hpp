#pragma once

#include "FpsMeter.hpp"

namespace ek {

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