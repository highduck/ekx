#include "particle_system.h"

#include "particle_decl.h"
#include <ek/scenex/particles/particle_emitter.h>
#include <ek/scenex/components/transform_2d.h>
#include <ek/scenex/particles/particle_layer.h>
#include <ek/util/locator.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/scenex/systems/game_time.h>

namespace ek {

particle_layer_t& find_particle_layer(ecs::entity e) {
    auto l = ecs::get_or_default<particle_emitter_t>(e).layer;
    if (!l) {
        l = e;
    }
    return ecs::get_or_create<particle_layer_t>(l);
}

void add_particle(particle_layer_t& layer, particle* particle_) {
    if (particle_) {
        layer.particles.push_back(particle_);
        particle_->init();
    }
}

particle* produce_particle(const particle_decl& decl) {
    auto* p = new particle;
    p->sprite = decl.sprite;
    p->reflector = decl.use_reflector;
    p->acc = decl.acceleration;
    p->alpha_mode = decl.alpha_mode;
    p->scale_mode = decl.scale_mode;
    p->font = decl.font;
    p->font_size = decl.font_size;
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
    p->color_functor = decl.color_func;
    return p;
}

void particles_burst(ecs::entity e, int count) {
    auto& emitter = ecs::get<particle_emitter_t>(e);
    const auto& data = ecs::get<particle_emitter_t>(e).data;
    const auto& position = ecs::get_or_default<transform_2d>(e).matrix.position() + emitter.position;
    float a = data.dir.random();
    asset_t<particle_decl> decl{emitter.particle};
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
    for (auto e : ecs::view<particle_emitter_t>()) {
        auto dt = get_delta_time(e);
        auto& emitter = ecs::get<particle_emitter_t>(e);
        if (!emitter.enabled || emitter.particle.empty()) {
            continue;
        }

        auto& layer = find_particle_layer(e);
        const auto& position = ecs::get_or_default<transform_2d>(e).matrix.position();

        emitter.time += dt;
        const auto& data = emitter.data;
        if (emitter.time >= data.interval) {
            asset_t<particle_decl> decl{emitter.particle};
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

particle* spawn_particle(ecs::entity e, const std::string& particle_id) {
    asset_t<particle_decl> decl{particle_id};
    if (decl) {
        auto* p = produce_particle(*decl);
        auto& to_layer = ecs::get_or_create<particle_layer_t>(e);
        add_particle(to_layer, p);
        return p;
    }
    return nullptr;
}

void update_particles() {
    static std::vector<particle*> alive_particles;
    for (auto e : ecs::view<particle_layer_t>()) {
        auto dt = get_delta_time(e);
        auto& layer = ecs::get<particle_layer_t>(e);
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

void draw_particle_layer(ecs::entity e) {
    auto& layer = ecs::get<particle_layer_t>(e);
    auto& drawer = resolve<drawer_t>();

    if (layer.cycled) {
        for (auto* p : layer.particles) {
            p->draw_cycled(drawer);
        }
    } else {
        for (auto* p : layer.particles) {
            p->draw(drawer, 0);
        }
    }
}

}