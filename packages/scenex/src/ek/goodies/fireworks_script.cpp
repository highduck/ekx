#include "fireworks_script.h"


#include <ek/util/ServiceLocator.hpp>
#include <ek/scenex/SceneFactory.hpp>
#include <ek/scenex/particles/ParticleSystem.hpp>
#include <ek/scenex/AudioManager.hpp>
#include <ek/scenex/2d/LayoutRect.hpp>
#include <ek/scenex/base/Node.hpp>

namespace ek {

void fireworks_script::start() {
    entity_.assign<ParticleLayer2D>();
    Display2D::make<ParticleRenderer2D>(entity_).target = ecs::EntityRef{entity_};
    auto& emitter = entity_.assign<ParticleEmitter2D>();
    emitter.data.burst = 0;
    emitter.particle = R_PARTICLE(H("firework_star"));
    emitter.layer = ecs::EntityRef{entity_};
    setTouchable(entity_, false);
}

void fireworks_script::update(float dt) {
    if (!enabled) {
        return;
    }

    timer_ -= dt;
    if (timer_ <= 0) {
        auto& emitter = entity_.get<ParticleEmitter2D>();

        auto rect = find_parent_layout_rect(entity_, true);
        rect.h *= 0.5f;

        emitter.position = rect.position + rect.size * vec2(random_f(), random_f());
        Locator::ref<AudioManager>().play_sound(H("sfx/firework"), random_range_f(0.5f, 1.0f));
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
        particles_burst(entity_, random_range_i(20, 30));

        emitter.particle = R_PARTICLE(H("firework_star"));
        emitter.data.speed.set(10, 100);
        emitter.data.acc.set(0, -50);
        particles_burst(entity_, random_range_i(60, 80));

        timer_ = random_range_f(0.1f, 1.0f);
//            _timer = random_range_f(0.01f, 0.1f);
    }
}

void fireworks_script::reset() {
    entity_.get<ParticleLayer2D>().particles.clear();
    timer_ = 0.0f;
}
}