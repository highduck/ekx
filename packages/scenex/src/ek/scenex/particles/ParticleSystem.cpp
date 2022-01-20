#include "ParticleSystem.hpp"

#include "ParticleDecl.hpp"
#include "Particle.hpp"
#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/util/ServiceLocator.hpp>

namespace ek {

ParticleLayer2D& find_particle_layer(ecs::EntityApi e) {
    auto l = e.get_or_default<ParticleEmitter2D>().layer.ent();
    if (!l) {
        l = e;
    }
    return l.get_or_create<ParticleLayer2D>();
}

Particle& produce_particle(ParticleLayer2D& toLayer, const ParticleDecl* decl) {
    auto& p = toLayer.particles.emplace_back();
    p.sprite = decl->sprite;
    p.reflector = decl->use_reflector;
    p.acc = decl->acceleration;
    p.alpha_mode = decl->alpha_mode;
    p.scale_mode = decl->scale_mode;
    p.acc_x_phase = decl->acc_x_phase.random();
    p.acc_x_speed = decl->acc_x_speed.random();
    p.scale_off_time = decl->scale_off_time;
    p.scale_start = decl->scale_start.random();
    p.scale_end = decl->scale_end.random();
    p.color = decl->color.next();
    p.angle_velocity_factor = decl->angle_velocity_factor;
    p.angle_base = decl->angle_base;
    p.rotation_speed = decl->rotation_speed;
    p.rotation = decl->rotation.random();
    p.alpha = decl->alpha_start.random();
    p.set_life_time(decl->life_time.random());
    p.offset = decl->color_offset;
    p.offset.a = unorm8_f32_clamped(decl->additive);
    return p;
}

void particles_burst(ecs::EntityApi e, int count, vec2_t relativeVelocity) {
    if(count < 0) {
        return;
    }
    const auto& emitter = e.get<ParticleEmitter2D>();
    const auto& data = emitter.data;
    auto layerEntity = emitter.layer.get();
    auto& layer = layerEntity.get<ParticleLayer2D>();
    const auto position = Transform2D::localToLocal(e, layerEntity, emitter.position);
    float a = data.dir.random();
    auto* decl = &REF_RESOLVE(res_particle, emitter.particle);
    while (count > 0) {
        auto& p = produce_particle(layer, decl);
        vec2_t pos = position;
        pos.x += random_range_f(data.rect.x, RECT_R(data.rect));
        pos.y += random_range_f(data.rect.y, RECT_B(data.rect));
        p.position = pos;
        float speed = data.speed.random();
        float acc = data.acc.random();
        const vec2_t dir = vec2_cs(a);
        p.velocity = speed * dir + relativeVelocity;
        p.acc += dir * acc;
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
        if (!emitter.enabled || !emitter.particle || !emitter.layer.invalidate()) {
            continue;
        }
        const auto dt = emitter.timer->dt;
        emitter.time += dt;
        const auto& data = emitter.data;
        if (emitter.time >= data.interval) {
            auto layerEntity = emitter.layer.get();
            auto& layer = layerEntity.get<ParticleLayer2D>();
            auto position = data.offset;
            position = Transform2D::localToLocal(e, layerEntity, position);

            auto* decl = &REF_RESOLVE(res_particle, emitter.particle);
            int count = data.burst;
            float a = data.dir.random();
            while (count > 0) {
                auto& p = produce_particle(layer, decl);
                vec2_t pos = position + data.offset;
                pos.x += random_range_f(data.rect.x, RECT_R(data.rect));
                pos.y += random_range_f(data.rect.y, RECT_B(data.rect));
                p.position = pos;
                float speed = data.speed.random();
                float acc = data.acc.random();
                const vec2_t dir = vec2_cs(a);
                p.velocity = speed * dir;
                p.acc = acc * dir;

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

Particle* spawn_particle(ecs::EntityApi e, string_hash_t particle_id) {
    ParticleDecl* decl = &RES_NAME_RESOLVE(res_particle, particle_id);
    auto& to_layer = e.get_or_create<ParticleLayer2D>();
    return &produce_particle(to_layer, decl);
}

void spawnFromEmitter(ecs::EntityApi src, ecs::EntityApi toLayer, const ParticleDecl* decl, ParticleEmitter2D& emitter,
                      int count) {
    if (count <= 0) {
        return;
    }
    const auto& data = emitter.data;
    auto a = data.dir.random();
    auto& layerComp = toLayer.get<ParticleLayer2D>();
    while (count > 0) {
        auto& p = produce_particle(layerComp, decl);
        const vec2_t position = data.offset + data.rect.position + data.rect.size * vec2(random_f(), random_f());
        const vec2_t dir = vec2_cs(a);
        const auto speed = data.speed.random();
        const auto acc = data.acc.random();
        p.position = Transform2D::localToLocal(src, toLayer, position);
        p.velocity = dir * speed + emitter.velocity;
        p.acc += dir * acc;
        if (emitter.on_spawn) {
            // TODO: EMITTER ENTITY!? and not `src` entity or `layer` entity?
            emitter.on_spawn(src, p);
        }

        --count;
        a += data.burst_rotation_delta.random();
    }
}

void update_particles() {
    for (auto e : ecs::view<ParticleLayer2D>()) {
        auto& layer = e.get<ParticleLayer2D>();
        auto dt = layer.timer->dt;
        auto& particles = layer.particles;
        uint32_t i = 0;
        while(i < particles.size()) {
            auto& p = particles[i];
            p.update(dt);
            if (p.is_alive()) {
                ++i;
            } else {
                particles.swapRemove(i);
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