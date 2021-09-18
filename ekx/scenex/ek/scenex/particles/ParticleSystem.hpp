#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/timers.hpp>
#include "ParticleDecl.hpp"
#include "Particle.hpp"

namespace ek {

struct ParticleLayer2D {
    Array<Particle> particles;
    bool keepAlive = false;
    TimeLayer timer;
};

EK_DECLARE_TYPE(ParticleLayer2D);

struct ParticleEmitter2D {
    typedef void SpawnCallback(ecs::EntityApi, Particle&);

    EmitterData data;
    float2 position = float2::zero;
    float2 velocity = float2::zero;
    SpawnCallback* on_spawn = nullptr;
    Res<ParticleDecl> particle;
    ecs::EntityRef layer;
    float time = 0.0f;
    TimeLayer timer;
    bool enabled = true;
};

EK_DECLARE_TYPE(ParticleEmitter2D);

class ParticleRenderer2D : public Drawable2D<ParticleRenderer2D> {
public:
    ParticleRenderer2D() = default;

    explicit ParticleRenderer2D(ecs::EntityRef target_) : target{target_} {}

    explicit ParticleRenderer2D(ecs::EntityApi target_) : target{target_} {}

    void draw() override;

    [[nodiscard]]
    rect_f getBounds() const override {
        return rect_f{};
    }

    [[nodiscard]]
    bool hitTest(float2 pos) const override {
        (void) pos;
        return false;
    }

public:
    ecs::EntityRef target{};
};

EK_DECLARE_TYPE(ParticleRenderer2D);

void particles_burst(ecs::EntityApi e, int count, float2 relativeVelocity = float2::zero);

Particle* spawn_particle(ecs::EntityApi e, const std::string& particle_id);
void spawnFromEmitter(ecs::EntityApi src, ecs::EntityApi toLayer, const ParticleDecl& decl, ParticleEmitter2D& emitter, int count);

void update_emitters();

void update_particles();

void draw_particle_layer(ecs::EntityApi e);

ParticleLayer2D& find_particle_layer(ecs::EntityApi e);
Particle& produce_particle(ParticleLayer2D& toLayer, const ParticleDecl& decl);

}


