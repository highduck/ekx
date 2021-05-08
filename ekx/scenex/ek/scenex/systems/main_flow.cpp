#include "main_flow.hpp"

#include <ek/util/ServiceLocator.hpp>

#include <ek/scenex/AudioManager.hpp>
#include <ek/scenex/particles/ParticleSystem.hpp>
#include <ek/scenex/InteractionSystem.hpp>
#include <ek/scenex/base/DestroyTimer.hpp>
#include <ek/goodies/Shake.hpp>
#include <ek/goodies/helpers/Trail2D.hpp>
#include <ek/goodies/bubble_text.hpp>
#include <ek/goodies/PopupManager.hpp>
#include <ek/timers.hpp>
#include <ek/scenex/2d/Camera2D.hpp>
#include <ek/scenex/2d/LayoutRect.hpp>
#include <ek/scenex/2d/Button.hpp>
#include <ek/scenex/2d/Viewport.hpp>
#include <ek/scenex/2d/MovieClip.hpp>
#include <ek/scenex/base/Tween.hpp>
#include <ek/goodies/GameScreen.hpp>
#include <ek/scenex/2d/DynamicAtlas.hpp>
#include <Tracy.hpp>

namespace ek {

using namespace ecs;

void scene_pre_update(EntityApi root, float dt) {
    ZoneScoped;

    Locator::ref<InteractionSystem>().process();

    TimeLayer::updateTimers(dt);

    Locator::ref<AudioManager>().update(dt);

    {
        auto* screenManager = Locator::get<GameScreenManager>();
        if (screenManager) {
            screenManager->update();
        }
    }
    LayoutRect::updateAll();
    Tween::updateAll();
    Shake::updateAll();

    BubbleText::updateAll();
    PopupManager::updateAll();

    updateScripts();
    Button::updateAll();
    MovieClip::updateAll();
}

void scene_post_update(ecs::EntityApi root) {
    ZoneScoped;

    DestroyTimer::updateAll();

    updateWorldTransformAll2(&ecs::the_world, root);

    Trail2D::updateAll();
    update_emitters();
    update_particles();
    Camera2D::updateQueue();
}

void scene_render(ecs::EntityApi root) {
    ZoneScoped;

    for (auto& it : Res<DynamicAtlas>::map()) {
        auto* atlas = const_cast<DynamicAtlas*>(it.second->content);
        if (atlas) {
            atlas->invalidate();
        }
    }

    Camera2D::render();
//    drawScene2D(root);
    //drawSceneGizmos(root);

//    for (auto& it : Res<DynamicAtlas>::map()) {
//        auto* atlas = const_cast<DynamicAtlas*>(it.second->content);
//        if (atlas) {
//            atlas->reset();
//        }
//    }
}

}


