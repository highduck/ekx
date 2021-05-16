#pragma once

namespace ek {

class FpsMeter final {
public:
    void update(float dt);

    [[nodiscard]]
    float getAverageFPS() const {
        return avgFPS_;
    }

private:
    // if less or equal 0 - calculate FPS for every frame
    const float measurementsPerSeconds = 2.0f;
    float avgFPS_ = 0.0f;
    float counter_ = 0.0f;
    float accum_ = 0.0f;
};

}