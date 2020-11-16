#include "game_time.hpp"

#include <ek/scenex/components/node.hpp>

namespace ek {

float updateState(TimeLayerState& layer, float dt_) {
    auto dt1 = dt_ * layer.scale;
    layer.dt = dt1;
    layer.total += dt1;
    return dt1;
}

TimeLayerState layers[4]{};

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

TimeLayerState* TimeLayer::operator->() {
    return layers + id;
}

const TimeLayerState* TimeLayer::operator->() const {
    return layers + id;
}

TimeLayer TimeLayer::Root{0};
TimeLayer TimeLayer::Game{1};
TimeLayer TimeLayer::HUD{2};
TimeLayer TimeLayer::UI{3};

}