#pragma once

#include "ParticleDecl.hpp"
#include "Particle.hpp"
#include <ecxx/ecxx.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ekx/app/time_layers.h>

namespace ek {

struct ParticleLayer2D {
    PodArray<Particle> particles;//{64};
    bool keepAlive = false;
    TimeLayer timer;
};

struct ParticleEmitter2D {
    typedef void SpawnCallback(ecs::EntityApi, Particle&);

    EmitterData data;
    vec2_t position = {};
    vec2_t velocity = {};
    SpawnCallback* on_spawn = nullptr;
    R(ParticleDecl) particle;
    ecs::EntityRef layer;
    float time = 0.0f;
    TimeLayer timer;
    bool enabled = true;
};

class ParticleRenderer2D {
public:
    ParticleRenderer2D() = default;

    explicit ParticleRenderer2D(ecs::EntityRef target_) : target{target_} {}

    explicit ParticleRenderer2D(ecs::EntityApi target_) : target{target_} {}

    void draw();

public:
    ecs::EntityRef target{};
};

ParticleRenderer2D* particle_renderer2d_setup(entity_t e);
void particle_renderer2d_draw(entity_t e);

void particles_burst(ecs::EntityApi e, int count, vec2_t relativeVelocity = {});

Particle* spawn_particle(ecs::EntityApi e, string_hash_t particle_id);

void spawnFromEmitter(ecs::EntityApi src, ecs::EntityApi toLayer, const ParticleDecl* decl, ParticleEmitter2D& emitter,
                      int count);

void update_emitters();

void update_particles();

void draw_particle_layer(ecs::EntityApi e);

ParticleLayer2D& find_particle_layer(ecs::EntityApi e);

Particle& produce_particle(ParticleLayer2D& toLayer, const ParticleDecl* decl);

}

ECX_COMP_TYPE_CXX(ek::ParticleLayer2D)
