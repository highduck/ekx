#pragma once

#include "ParticleDecl.hpp"
#include "Particle.hpp"
#include <ecxx/ecxx.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/base/TimeLayer.hpp>

namespace ek {

struct ParticleLayer2D {
    Array<Particle> particles;
    bool keepAlive = false;
    TimeLayer timer;
};

ECX_TYPE(19, ParticleLayer2D);

struct ParticleEmitter2D {
    typedef void SpawnCallback(ecs::EntityApi, Particle&);

    EmitterData data;
    Vec2f position = Vec2f::zero;
    Vec2f velocity = Vec2f::zero;
    SpawnCallback* on_spawn = nullptr;
    Res<ParticleDecl> particle;
    ecs::EntityRef layer;
    float time = 0.0f;
    TimeLayer timer;
    bool enabled = true;
};

ECX_TYPE(18, ParticleEmitter2D);

class ParticleRenderer2D : public Drawable2D<ParticleRenderer2D> {
public:
    ParticleRenderer2D() = default;

    explicit ParticleRenderer2D(ecs::EntityRef target_) : target{target_} {}

    explicit ParticleRenderer2D(ecs::EntityApi target_) : target{target_} {}

    void draw() override;

    [[nodiscard]]
    Rect2f getBounds() const override {
        return Rect2f{};
    }

    [[nodiscard]]
    bool hitTest(Vec2f pos) const override {
        (void) pos;
        return false;
    }

public:
    ecs::EntityRef target{};
};

EK_DECLARE_TYPE(ParticleRenderer2D);
EK_TYPE_INDEX_T(IDrawable2D, ParticleRenderer2D, IDrawable2D_ParticleRenderer2D);

void particles_burst(ecs::EntityApi e, int count, Vec2f relativeVelocity = Vec2f::zero);

Particle* spawn_particle(ecs::EntityApi e, const char* particle_id);

void spawnFromEmitter(ecs::EntityApi src, ecs::EntityApi toLayer, const ParticleDecl& decl, ParticleEmitter2D& emitter,
                      int count);

void update_emitters();

void update_particles();

void draw_particle_layer(ecs::EntityApi e);

ParticleLayer2D& find_particle_layer(ecs::EntityApi e);

Particle& produce_particle(ParticleLayer2D& toLayer, const ParticleDecl& decl);

}


