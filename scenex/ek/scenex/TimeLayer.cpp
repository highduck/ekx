#include "TimeLayer.hpp"

#include <ek/scenex/base/Node.hpp>

namespace ek {

float updateState(TimeLayer::State& layer, float dt_) {
    auto dt1 = dt_ * layer.scale;
    layer.dt = dt1;
    layer.total += dt1;
    return dt1;
}

TimeLayer::State layers[4]{};

const auto MAX_DELTA_TIME = 0.3;

void TimeLayer::updateTimers(float raw) {
    if (raw > MAX_DELTA_TIME) {
        raw = MAX_DELTA_TIME;
    }
    auto dt = updateState(layers[0], raw);
    updateState(layers[1], dt);
    updateState(layers[2], dt);
    updateState(layers[3], dt);
}

TimeLayer::State* TimeLayer::operator->() {
    return layers + id;
}

const TimeLayer::State* TimeLayer::operator->() const {
    return layers + id;
}

TimeLayer TimeLayer::Root{0};
TimeLayer TimeLayer::Game{1};
TimeLayer TimeLayer::HUD{2};
TimeLayer TimeLayer::UI{3};

}