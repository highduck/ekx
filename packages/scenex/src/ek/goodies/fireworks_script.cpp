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
    emitter.particle.setID("firework_star");
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

        emitter.position = {
                random(rect.x, RECT_R(rect)),
                random(rect.y, RECT_B(rect))
        };

        Locator::ref<AudioManager>().play_sound("sfx/firework", random(0.5f, 1.0f));
        Res<ParticleDecl> part{"firework_star"};
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

        emitter.particle.setID("firework_spark");
        emitter.data.acc.set(0, 100);
        emitter.data.speed.set(50, 100);
        particles_burst(entity_, random_int(20, 30));

        emitter.particle.setID("firework_star");
        emitter.data.speed.set(10, 100);
        emitter.data.acc.set(0, -50);
        particles_burst(entity_, random_int(60, 80));

        timer_ = random(0.1f, 1.0f);
//            _timer = random(0.01f, 0.1f);
    }
}

void fireworks_script::reset() {
    entity_.get<ParticleLayer2D>().particles.clear();
    timer_ = 0.0f;
}
}