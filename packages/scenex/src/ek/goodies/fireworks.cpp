#include "fireworks.h"

#include <ek/scenex/SceneFactory.hpp>
#include <ek/scenex/particles/ParticleSystem.hpp>
#include <ekx/app/audio_manager.h>
#include <ek/scenex/2d/LayoutRect.hpp>
#include <ek/scenex/base/Node.hpp>

static fireworks_state_t fireworks;

void start_fireworks(entity_t e) {
    using ek::ParticleLayer2D;
    using ek::ParticleRenderer2D;
    using ek::ParticleEmitter2D;
    using ek::Display2D;
    using ek::set_touchable;

    ecs::add<ParticleLayer2D>(e);
    ek::particle_renderer2d_setup(e)->target = e;
    auto& emitter = ecs::add<ParticleEmitter2D>(e);
    emitter.data.burst = 0;
    emitter.particle = R_PARTICLE(H("firework_star"));
    emitter.layer = e;
    emitter.enabled = true;
    set_touchable(e, false);
}

void update_fireworks() {
    using ek::ParticleEmitter2D;
    using ek::find_parent_layout_rect;
    using ek::res_particle;
    using ek::ParticleDecl;
    using ek::particles_burst;

    auto e = fireworks.layer;
    if(!is_entity(e) || !fireworks.enabled) {
        return;
    }
    fireworks.timer_ -= g_time_layers[fireworks.time_layer].dt;
    if (fireworks.timer_ <= 0) {
        auto& emitter = ecs::get<ParticleEmitter2D>(e);

        auto rect = find_parent_layout_rect(e, true);
        rect.h *= 0.5f;

        emitter.position = rect.position + rect.size * vec2(random_f(), random_f());
        play_sound(H("sfx/firework"), random_range_f(0.5f, 1.0f));
        ParticleDecl* part = &RES_NAME_RESOLVE(res_particle, H("firework_star"));
        switch (random_n(4)) {
            case 0:
                part->color.set_gradient(ARGB(0xffffff00), ARGB(0xffff7f00));
                break;
            case 1:
                part->color.set_gradient(ARGB(0xfffea7f9), ARGB(0xffff006c));
                break;
            case 2:
                part->color.set_gradient(ARGB(0xffd5fdfd), ARGB(0xff00c0ff));
                break;
            case 3:
                part->color.set_gradient(ARGB(0xff2e0678), ARGB(0xffb066cf));
                break;
            default:
                break;
        }

        emitter.particle = R_PARTICLE(H("firework_spark"));
        emitter.data.acc.set(0, 100);
        emitter.data.speed.set(50, 100);
        particles_burst(e, random_range_i(20, 30));

        emitter.particle = R_PARTICLE(H("firework_star"));
        emitter.data.speed.set(10, 100);
        emitter.data.acc.set(0, -50);
        particles_burst(e, random_range_i(60, 80));

        fireworks.timer_ = random_range_f(0.1f, 1.0f);
//            _timer = random_range_f(0.01f, 0.1f);
    }
}

void stop_fireworks() {
    using ek::ParticleLayer2D;
    entity_t e = fireworks.layer;
    if(is_entity(e)) {
        ParticleLayer2D* layer = ecs::try_get<ParticleLayer2D>(e);
        if (layer) {
            layer->particles.clear();
        }
    }
    fireworks.layer = NULL_ENTITY;
    fireworks.timer_ = 0.0f;
}
