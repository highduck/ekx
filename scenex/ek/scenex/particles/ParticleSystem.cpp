#include "ParticleSystem.hpp"

#include "ParticleDecl.hpp"
#include "Particle.hpp"
#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/util/locator.hpp>

namespace ek {

ParticleLayer2D& find_particle_layer(ecs::entity e) {
    auto l = e.get_or_default<ParticleEmitter2D>().layer.ent();
    if (!l) {
        l = e;
    }
    return l.get_or_create<ParticleLayer2D>();
}

Particle& produce_particle(ParticleLayer2D& toLayer, const ParticleDecl& decl) {
    auto& p = toLayer.particles.emplace_back();
    p.sprite = decl.sprite;
    p.reflector = decl.use_reflector;
    p.acc = decl.acceleration;
    p.alpha_mode = decl.alpha_mode;
    p.scale_mode = decl.scale_mode;
    p.acc_x_phase = decl.acc_x_phase.random();
    p.acc_x_speed = decl.acc_x_speed.random();
    p.scale_off_time = decl.scale_off_time;
    p.scale_start = decl.scale_start.random();
    p.scale_end = decl.scale_end.random();
    p.color = decl.color.next();
    p.angle_velocity_factor = decl.angle_velocity_factor;
    p.angle_base = decl.angle_base;
    p.rotation_speed = decl.rotation_speed;
    p.rotation = decl.rotation.random();
    p.alpha = decl.alpha_start.random();
    p.set_life_time(decl.life_time.random());
    p.offset = decl.color_offset;
    p.offset.af(decl.additive);
    return p;
}

void particles_burst(ecs::entity e, int count) {
    auto& emitter = e.get<ParticleEmitter2D>();
    const auto& data = e.get<ParticleEmitter2D>().data;
    const auto position = e.get_or_default<Transform2D>().getPosition() + emitter.position;
    float a = data.dir.random();
    Res<ParticleDecl> decl{emitter.particle};
    auto& layer = find_particle_layer(e);
    while (count > 0) {
        auto& p = produce_particle(layer, *decl);
        float2 pos = position;
        pos.x += random(data.rect.x, data.rect.right());
        pos.y += random(data.rect.y, data.rect.bottom());
        p.position = pos;
        float speed = data.speed.random();
        float acc = data.acc.random();
        float dx = cosf(a);
        float dy = sinf(a);
        p.velocity = float2(dx * speed, dy * speed);
        p.acc = float2(p.acc.x + dx * acc, p.acc.y + dy * acc);
        if (emitter.on_spawn) {
            emitter.on_spawn(e, p);
        }
        --count;
        a += data.burst_rotation_delta.random();
    }
}

void update_emitters() {
    for (auto e : ecs::view<ParticleEmitter2D>()) {
        auto& emitter = e.get<ParticleEmitter2D>();
        auto dt = emitter.timer->dt;
        if (!emitter.enabled || emitter.particle.empty()) {
            continue;
        }

        auto& layer = find_particle_layer(e);
        auto position = e.get_or_default<Transform2D>().getPosition();

        emitter.time += dt;
        const auto& data = emitter.data;
        if (emitter.time >= data.interval) {
            Res<ParticleDecl> decl{emitter.particle};
            int count = data.burst;
            float a = data.dir.random();
            while (count > 0) {
                auto& p = produce_particle(layer, *decl);
                float x = 0.0f;
                float y = 0.0f;
                x += position.x;
                y += position.y;
                x += data.offset.x;
                y += data.offset.y;
                x += random(data.rect.x, data.rect.right());
                y += random(data.rect.y, data.rect.bottom());

                p.position = float2(x, y);
                float speed = data.speed.random();
                float acc = data.acc.random();
                float dx = cosf(a);
                float dy = sinf(a);
                p.velocity = float2(dx * speed, dy * speed);
                p.acc = float2(dx * acc, dy * acc);

                if (emitter.on_spawn) {
                    emitter.on_spawn(e, p);
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
        auto& to_layer = e.get_or_create<ParticleLayer2D>();
        return &produce_particle(to_layer, *decl);
    }
    return nullptr;
}

void update_particles() {
    for (auto e : ecs::view<ParticleLayer2D>()) {
        auto& layer = e.get<ParticleLayer2D>();
        auto dt = layer.timer->dt;
        auto size = static_cast<int>(layer.particles.size());
        for (int i = 0; i < size;) {
            auto& p = layer.particles[i];
            p.update(dt);
            if (p.is_alive()) {
                ++i;
            } else {
                layer.particles[i] = layer.particles.back();
                layer.particles.pop_back();
                --size;
            }
        }
    }
}

void ParticleRenderer2D::draw() {
    if (target) {
        auto* layer = target.get().tryGet<ParticleLayer2D>();
        if (layer) {
            for (auto& p : layer->particles) {
                p.draw();
            }
        }
    }
}

}