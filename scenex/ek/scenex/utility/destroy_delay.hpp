#pragma once

#include <ecxx/ecxx.hpp>

namespace ek {

struct destroy_delay_t {
    float delay = 0.0f;
};

void destroy_delayed_entities(float dt);

inline void destroy_delay(ecs::entity e, float delay = 0.0f) {
    ecs::get_or_create<destroy_delay_t>(e) = {delay};
}

}


