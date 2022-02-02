#include "fireworks.h"

#include <ek/scenex/SceneFactory.hpp>
#include <ek/scenex/particles/ParticleSystem.hpp>
#include <ekx/app/audio_manager.h>
#include <ek/scenex/2d/LayoutRect.hpp>
#include <ek/scenex/base/Node.hpp>

void setup_fireworks(ecs::EntityApi e) {
    using ek::ParticleLayer2D;
    using ek::ParticleRenderer2D;
    using ek::ParticleEmitter2D;
    using ek::Display2D;
    using ek::setTouchable;
    e.assign<fireworks_comp>();
    e.assign<ParticleLayer2D>();

    ek::particle_renderer2d_setup(e.index)->target = ecs::EntityRef{e};
    auto& emitter = e.assign<ParticleEmitter2D>();
    emitter.data.burst = 0;
    emitter.particle = R_PARTICLE(H("firework_star"));
    emitter.layer = ecs::EntityRef{e};
    setTouchable(e, false);
}

void update_fireworks(float dt) {
    using ek::ParticleEmitter2D;
    using ek::find_parent_layout_rect;
    using ek::res_particle;
    using ek::ParticleDecl;
    using ek::particles_burst;

    for (auto e: ecs::view<fireworks_comp>()) {
        auto& d = e.get<fireworks_comp>();
        if (!d.enabled) {
            continue;
        }

        d.timer_ -= dt;
        if (d.timer_ <= 0) {
            auto& emitter = e.get<ParticleEmitter2D>();

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

            d.timer_ = random_range_f(0.1f, 1.0f);
//            _timer = random_range_f(0.01f, 0.1f);
        }
    }
}

void reset_fireworks(ecs::EntityApi e) {
    using ek::ParticleLayer2D;
    ParticleLayer2D* layer = e.tryGet<ParticleLayer2D>();
    if (layer) {
        layer->particles.clear();
    }
    fireworks_comp* fw = e.tryGet<fireworks_comp>();
    if (fw) {
        fw->timer_ = 0.0f;
    }
}
