#pragma once

#include <ecxx/ecxx.hpp>
#include "particle_decl.hpp"
#include "particle_layer.hpp"

namespace ek {

class particle;

void particles_burst(ecs::entity e, int count);
particle* spawn_particle(ecs::entity e, const std::string& particle_id);

void update_emitters();
void update_particles();

void draw_particle_layer(ecs::entity e);
particle* produce_particle(const particle_decl& decl);
void add_particle(particle_layer_t& layer, particle* particle_);

}

