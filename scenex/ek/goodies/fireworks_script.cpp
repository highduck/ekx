#include "fireworks_script.h"

#include <ek/scenex/utility/scene_management.hpp>
#include <ek/scenex/particles/particle_system.hpp>
#include <ek/scenex/AudioManager.hpp>
#include <ek/util/locator.hpp>
#include <ek/scenex/components/layout.hpp>
#include <ek/math/box.hpp>

namespace ek {

void fireworks_script::start() {
    entity_.assign<ParticleLayer2D>();
    entity_.assign<Display2D>(new ParticleRenderer2D(entity_));
    auto& emitter = entity_.assign<ParticleEmitter2D>();
    emitter.data.burst = 0;
    emitter.particle = "firework_star";
    set_touchable(entity_, false);
}

void fireworks_script::update(float dt) {
    if (!enabled) {
        return;
    }

    timer_ -= dt;
    if (timer_ <= 0) {
        auto& emitter = ecs::get<ParticleEmitter2D>(entity_);

        auto rect = find_parent_layout_rect(entity_, true);
        rect.height = rect.height * 0.5f;

        emitter.position = {
                random(rect.x, rect.right()),
                random(rect.y, rect.bottom())
        };

        resolve<AudioManager>().play_sound("sfx/firework", random(0.5f, 1.0f));
        asset_t<particle_decl> part{"firework_star"};
        switch (random_int(0, 3)) {
            case 0:
                part->color.set_gradient(0xffffff00_argb, 0xffff7f00_argb);
                break;
            case 1:
                part->color.set_gradient(0xfffea7f9_argb, 0xffff006c_argb);
                break;
            case 2:
                part->color.set_gradient(0xffd5fdfd_argb, 0xff00c0ff_argb);
                break;
            case 3:
                part->color.set_gradient(0xff2e0678_argb, 0xffb066cf_argb);
                break;
            default:
                break;
        }

        emitter.particle = "firework_spark";
        emitter.data.acc.set(0, 100);
        emitter.data.speed.set(50, 100);
        particles_burst(entity_, random_int(20, 30));

        emitter.particle = "firework_star";
        emitter.data.speed.set(10, 100);
        emitter.data.acc.set(0, -50);
        particles_burst(entity_, random_int(60, 80));

        timer_ = random(0.1f, 1.0f);
//            _timer = random(0.01f, 0.1f);
    }
}

void fireworks_script::reset() {
    ecs::get<ParticleLayer2D>(entity_).particles.clear();
    timer_ = 0.0f;
}
}