#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/timers.hpp>
#include <ek/ds/FreeList.hpp>
#include "ParticleDecl.hpp"
#include "Particle.hpp"

namespace ek {

struct ParticleLayer2D {
    std::vector<Particle> particles;
    bool keep_alive = false;
    TimeLayer timer;
};

struct ParticleEmitter2D {
    emitter_data data;
    float2 position = float2::zero;
    std::function<void(Particle&)> on_spawn;
    std::string particle;
    ecs::EntityRef layer;
    float time = 0.0f;
    TimeLayer timer;
    bool enabled = true;
};

class ParticleRenderer2D : public Drawable2D<ParticleRenderer2D> {
public:
    ParticleRenderer2D() = default;

    explicit ParticleRenderer2D(ecs::EntityRef target_) : target{target_} {}

    explicit ParticleRenderer2D(ecs::entity target_) : target{target_} {}

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

void particles_burst(ecs::entity e, int count);

Particle* spawn_particle(ecs::entity e, const std::string& particle_id);

void update_emitters();

void update_particles();

void draw_particle_layer(ecs::entity e);

Particle& produce_particle(ParticleLayer2D& toLayer, const ParticleDecl& decl);

}


