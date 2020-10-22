#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/scenex/particles/particle_decl.hpp>

namespace ek {

struct particle_emitter_t {
    emitter_data data;
    float2 position = float2::zero;
    std::function<void(particle&)> on_spawn;
    std::string particle;
    ecs::entity layer;
    float time = 0.0f;
    bool enabled = true;
};

}