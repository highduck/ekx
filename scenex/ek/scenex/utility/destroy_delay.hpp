#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/scenex/TimeLayer.hpp>

namespace ek {

struct destroy_delay_t {
    float delay = 0.0f;
    TimeLayer timer{};
};

void destroy_delayed_entities(float dt);

void destroy_delay(ecs::entity e, float delay = 0.0f, TimeLayer timer = {});

void destroy_children_delay(ecs::entity e, float delay = 0.0f, TimeLayer timer = {});

}


