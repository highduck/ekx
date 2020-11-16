#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/game_time.hpp>
#include "particle_decl.hpp"

namespace ek {

class particle;

struct ParticleLayer2D {
    std::vector<particle*> particles;
    bool keep_alive = false;
    TimeLayer timer;
};

struct ParticleEmitter2D {
    emitter_data data;
    float2 position = float2::zero;
    std::function<void(particle&)> on_spawn;
    std::string particle;
    ecs::entity layer;
    float time = 0.0f;
    TimeLayer timer;
    bool enabled = true;
};

class ParticleRenderer2D : public Drawable2D<ParticleRenderer2D> {
public:
    ParticleRenderer2D() = default;

    explicit ParticleRenderer2D(ecs::entity target_) :
            target{target_} {

    }

    void draw() override;

    [[nodiscard]] rect_f getBounds() const override { return rect_f{}; }

    [[nodiscard]] bool hitTest(float2 pos) const override { return false; }

public:
    ecs::entity target{};
    bool cycled = false;
};

void particles_burst(ecs::entity e, int count);

particle* spawn_particle(ecs::entity e, const std::string& particle_id);

void update_emitters();

void update_particles();

void draw_particle_layer(ecs::entity e);

particle* produce_particle(const particle_decl& decl);

void add_particle(ParticleLayer2D& layer, particle* particle_);

}


