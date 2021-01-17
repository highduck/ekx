#include "main_flow.hpp"

#include <ek/util/locator.hpp>

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
#include <ek/scenex/2d/Canvas.hpp>
#include <ek/scenex/2d/MovieClip.hpp>
#include <ek/scenex/base/Tween.hpp>
#include <ek/goodies/GameScreen.hpp>
#include <ek/scenex/2d/DynamicAtlas.hpp>
#include <Tracy.hpp>

namespace ek {

using namespace ecs;

void scene_pre_update(entity root, float dt) {
    ZoneScoped;

    resolve<InteractionSystem>().process();

    TimeLayer::updateTimers(dt);

    resolve<AudioManager>().update(dt);

    {
        auto* screenManager = try_resolve<GameScreenManager>();
        if (screenManager) {
            screenManager->update();
        }
    }
    Canvas::updateAll();
    LayoutRect::updateAll();
    Tween::updateAll();
    Shake::updateAll();

    BubbleText::updateAll();
    PopupManager::updateAll();

    updateScripts();
    Button::updateAll();
    MovieClip::updateAll();
}

void scene_post_update(ecs::entity root) {
    ZoneScoped;

    DestroyTimer::updateAll();

    updateWorldTransformAll(&ecs::the_world, root);

    Trail2D::updateAll();
    update_emitters();
    update_particles();
    Camera2D::updateQueue();
}

void scene_render(ecs::entity root) {
    ZoneScoped;

    Camera2D::render();
//    drawScene2D(root);
    //drawSceneGizmos(root);

    for (auto& it : Res<DynamicAtlas>::map()) {
        auto* atlas = const_cast<DynamicAtlas*>(it.second->content);
        if (atlas) {
            atlas->invalidate();
        }
    }

//    for (auto& it : Res<DynamicAtlas>::map()) {
//        auto* atlas = const_cast<DynamicAtlas*>(it.second->content);
//        if (atlas) {
//            atlas->reset();
//        }
//    }
}

}


