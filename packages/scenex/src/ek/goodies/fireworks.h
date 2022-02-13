#pragma once

#include <ecx/ecx_fwd.hpp>
#include <ekx/app/time_layers.h>

struct fireworks_state_t {
    entity_t layer = NULL_ENTITY;
    TimeLayer time_layer = TIME_LAYER_ROOT;
    float timer_ = 1.0f;
    bool enabled = false;
};

void start_fireworks(entity_t e);

void update_fireworks();

void stop_fireworks();
