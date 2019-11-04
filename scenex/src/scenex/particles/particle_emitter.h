#pragma once

#include <scenex/config.h>
#include <scenex/config/ecs.h>
#include <scenex/particles/particle_decl.h>

namespace scenex {

struct particle_emitter_t {
    emitter_data data;
    ek::float2 position = float2::zero;
    std::function<void(particle&)> on_spawn;
    std::string particle;
    ecs::entity layer;
    float time = 0.0f;
    bool enabled = true;
};

}