#include "ParticleSystem.hpp"

#include "ParticleDecl.hpp"
#include "Particle.hpp"
#include <ek/scenex/2d/Transform2D.hpp>


namespace ek {

ParticleLayer2D& find_particle_layer(entity_t e) {
    auto l = ecs::get_or_default<ParticleEmitter2D>(e).layer;
    if (!is_entity(l)) {
        l = e;
    }
    return ecs::add<ParticleLayer2D>(l);
}

Particle& produce_particle(ParticleLayer2D& toLayer, const ParticleDecl* decl) {
    auto& p = toLayer.particles.emplace_back({});
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

void particles_burst(entity_t e, int count, vec2_t relativeVelocity) {
    if (count < 0) {
        return;
    }
    const auto& emitter = ecs::get<ParticleEmitter2D>(e);
    const auto& data = emitter.data;
    auto layer_entity = emitter.layer;
    auto& layer = ecs::get<ParticleLayer2D>(layer_entity);
    const auto position = local_to_local(e, layer_entity, emitter.position);
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
    for (auto e: ecs::view<ParticleEmitter2D>()) {
        auto& emitter = ecs::get<ParticleEmitter2D>(e);
        entity_t emitter_layer = emitter.layer;
        if (!emitter.enabled || !emitter.particle || !is_entity(emitter_layer)) {
            continue;
        }
        const auto dt = g_time_layers[emitter.timer].dt;
        emitter.time += dt;
        const auto& data = emitter.data;
        if (emitter.time >= data.interval) {
            auto& layer = ecs::get<ParticleLayer2D>(emitter_layer);
            auto position = data.offset;
            position = local_to_local(e, emitter_layer, position);

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

Particle* spawn_particle(entity_t e, string_hash_t particle_id) {
    ParticleDecl* decl = &RES_NAME_RESOLVE(res_particle, particle_id);
    auto& to_layer = ecs::add<ParticleLayer2D>(e);
    return &produce_particle(to_layer, decl);
}

void spawnFromEmitter(entity_t src, entity_t toLayer, const ParticleDecl* decl, ParticleEmitter2D& emitter,
                      int count) {
    if (count <= 0) {
        return;
    }
    const auto& data = emitter.data;
    auto a = data.dir.random();
    auto& layerComp = ecs::get<ParticleLayer2D>(toLayer);
    while (count > 0) {
        auto& p = produce_particle(layerComp, decl);
        const vec2_t position = data.offset + data.rect.position + data.rect.size * vec2(random_f(), random_f());
        const vec2_t dir = vec2_cs(a);
        const auto speed = data.speed.random();
        const auto acc = data.acc.random();
        p.position = local_to_local(src, toLayer, position);
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
    for (auto e: ecs::view<ParticleLayer2D>()) {
        auto& layer = ecs::get<ParticleLayer2D>(e);
        auto dt = g_time_layers[layer.timer].dt;
        auto& particles = layer.particles;
        uint32_t i = 0;
        while (i < particles.size()) {
            auto& p = particles[i];
            p.update(dt);
            if (p.is_alive()) {
                ++i;
            } else {
                particles.swap_remove(i);
            }
        }
    }
}

ParticleRenderer2D* particle_renderer2d_setup(entity_t e) {
    Display2D& disp = ecs::add<Display2D>(e);
    ParticleRenderer2D& pr = ecs::add<ParticleRenderer2D>(e);
    pr.target = e;
    disp.draw = particle_renderer2d_draw;
    return &pr;
}

void particle_renderer2d_draw(entity_t e) {
    ecs::get<ParticleRenderer2D>(e).draw();
}

void ParticleRenderer2D::draw() {
    entity_t target_entity = target;
    if (is_entity(target_entity)) {
        auto* layer = ecs::try_get<ParticleLayer2D>(target_entity);
        if (layer) {
            for (auto& p: layer->particles) {
                p.draw();
            }
        }
    }
}

}