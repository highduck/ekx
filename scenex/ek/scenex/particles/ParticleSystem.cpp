#include "ParticleSystem.hpp"

#include "ParticleDecl.hpp"
#include "Particle.hpp"
#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/util/locator.hpp>

namespace ek {

ParticleLayer2D& find_particle_layer(ecs::entity e) {
    auto l = ecs::get_or_default<ParticleEmitter2D>(e).layer;
    if (!l) {
        l = e;
    }
    return ecs::get_or_create<ParticleLayer2D>(l);
}

void add_particle(ParticleLayer2D& layer, Particle* particle_) {
    if (particle_) {
        layer.particles.push_back(particle_);
        particle_->init();
    }
}

Particle* produce_particle(const ParticleDecl& decl) {
    auto* p = new Particle;
    p->sprite = decl.sprite;
    p->reflector = decl.use_reflector;
    p->acc = decl.acceleration;
    p->alpha_mode = decl.alpha_mode;
    p->scale_mode = decl.scale_mode;
    p->acc_x_phase = decl.acc_x_phase.random();
    p->acc_x_speed = decl.acc_x_speed.random();
    p->scale_off_time = decl.scale_off_time;
    p->scale_start = decl.scale_start.random();
    p->scale_end = decl.scale_end.random();
    p->color = decl.color.next();
    p->angle_velocity_factor = decl.angle_velocity_factor;
    p->angle_base = decl.angle_base;
    p->rotation_speed = decl.rotation_speed;
    p->rotation = decl.rotation.random();
    p->alpha = decl.alpha_start.random();
    p->set_life_time(decl.life_time.random());
    p->offset = decl.color_offset;
    p->offset.af(decl.additive);
    return p;
}

void particles_burst(ecs::entity e, int count) {
    auto& emitter = ecs::get<ParticleEmitter2D>(e);
    const auto& data = ecs::get<ParticleEmitter2D>(e).data;
    const auto& position = ecs::get_or_default<Transform2D>(e).position + emitter.position;
    float a = data.dir.random();
    Res<ParticleDecl> decl{emitter.particle};
    auto& layer = find_particle_layer(e);
    while (count > 0) {
        auto* p = produce_particle(*decl);
        add_particle(layer, p);
        float2 pos = position;
        pos.x += random(data.rect.x, data.rect.right());
        pos.y += random(data.rect.y, data.rect.bottom());
        p->position = pos;
        float speed = data.speed.random();
        float acc = data.acc.random();
        float dx = cosf(a);
        float dy = sinf(a);
        p->velocity = float2(dx * speed, dy * speed);
        p->acc = float2(p->acc.x + dx * acc, p->acc.y + dy * acc);
        if (emitter.on_spawn) {
            emitter.on_spawn(*p);
        }
        --count;
        a += data.burst_rotation_delta.random();
    }
}

void update_emitters() {
    for (auto e : ecs::view<ParticleEmitter2D>()) {
        auto& emitter = ecs::get<ParticleEmitter2D>(e);
        auto dt = emitter.timer->dt;
        if (!emitter.enabled || emitter.particle.empty()) {
            continue;
        }

        auto& layer = find_particle_layer(e);
        const auto& position = ecs::get_or_default<Transform2D>(e).position;

        emitter.time += dt;
        const auto& data = emitter.data;
        if (emitter.time >= data.interval) {
            Res<ParticleDecl> decl{emitter.particle};
            int count = data.burst;
            float a = data.dir.random();
            while (count > 0) {
                auto* p = produce_particle(*decl);
                add_particle(layer, p);
                float x = 0.0f;
                float y = 0.0f;
                x += position.x;
                y += position.y;
                x += data.offset.x;
                y += data.offset.y;
                x += random(data.rect.x, data.rect.right());
                y += random(data.rect.y, data.rect.bottom());
                p->position = float2(x, y);
                float speed = data.speed.random();
                float acc = data.acc.random();
                float dx = cosf(a);
                float dy = sinf(a);
                p->velocity = float2(dx * speed, dy * speed);
                p->acc = float2(dx * acc, dy * acc);

                if (emitter.on_spawn) {
                    emitter.on_spawn(*p);
                }

                --count;
                a += data.burst_rotation_delta.random();
            }
            emitter.time = 0.0f;
        }
    }
}

Particle* spawn_particle(ecs::entity e, const std::string& particle_id) {
    Res<ParticleDecl> decl{particle_id};
    if (decl) {
        auto* p = produce_particle(*decl);
        auto& to_layer = ecs::get_or_create<ParticleLayer2D>(e);
        add_particle(to_layer, p);
        return p;
    }
    return nullptr;
}

void update_particles() {
    static std::vector<Particle*> alive_particles;
    for (auto e : ecs::view<ParticleLayer2D>()) {
        auto& layer = ecs::get<ParticleLayer2D>(e);
        auto dt = layer.timer->dt;
        alive_particles.clear();
        for (auto* p : layer.particles) {
            p->update(dt);
            if (p->is_alive()) {
                alive_particles.push_back(p);
            } else {
                delete p;
            }
        }
        layer.particles = alive_particles;
    }
}

void ParticleRenderer2D::draw() {
    if (target) {
        assert(target.valid());
        auto* layer = target.tryGet<ParticleLayer2D>();
        if (layer) {
            for (auto* p : layer->particles) {
                p->draw();
            }
        }
    }
}

}