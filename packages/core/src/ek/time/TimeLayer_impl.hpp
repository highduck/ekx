#pragma once

#include "TimeLayer.hpp"

namespace ek {

float updateState(TimeLayer::State& layer, float dt_) {
    auto dt1 = dt_ * layer.scale;
    layer.dt = dt1;
    layer.total += dt1;
    return dt1;
}

void TimeLayer::updateTimers(float raw) {
    if (raw > DeltaTimeMax) {
        raw = DeltaTimeMax;
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

}