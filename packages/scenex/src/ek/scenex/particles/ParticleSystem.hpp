#pragma once

#include "ParticleDecl.hpp"
#include "Particle.hpp"
#include <ecx/ecx.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/ds/PodArray.hpp>
#include <ekx/app/time_layers.h>

namespace ek {

struct ParticleLayer2D {
    PodArray<Particle> particles;//{64};
    bool keepAlive = false;
    TimeLayer timer;
};

struct ParticleEmitter2D {
    typedef void SpawnCallback(entity_t, Particle&);

    EmitterData data;
    vec2_t position = {};
    vec2_t velocity = {};
    SpawnCallback* on_spawn = nullptr;
    R(ParticleDecl) particle;
    entity_t layer = {};
    float time = 0.0f;
    TimeLayer timer;
    bool enabled = true;
};

struct ParticleRenderer2D {
    ParticleRenderer2D() = default;

    explicit ParticleRenderer2D(entity_t target_) : target{target_} {}

    void draw();

    entity_t target = NULL_ENTITY;
};

ParticleRenderer2D* particle_renderer2d_setup(entity_t e);
void particle_renderer2d_draw(entity_t e);

void particles_burst(entity_t e, int count, vec2_t relativeVelocity = {});

Particle* spawn_particle(entity_t e, string_hash_t particle_id);

void spawnFromEmitter(entity_t src, entity_t toLayer, const ParticleDecl* decl, ParticleEmitter2D& emitter,
                      int count);

void update_emitters();

void update_particles();

void draw_particle_layer(entity_t e);

ParticleLayer2D& find_particle_layer(entity_t e);

Particle& produce_particle(ParticleLayer2D& toLayer, const ParticleDecl* decl);

}

ECX_COMP_TYPE_CXX(ek::ParticleLayer2D)
