#pragma once

#include "Stopwatch.hpp"

namespace ek {

class FrameTimer final {
public:
    float deltaTime = 0.0;
    uint64_t frameIndex = 0;

    float update();

private:
    Stopwatch timer_{};
};

}