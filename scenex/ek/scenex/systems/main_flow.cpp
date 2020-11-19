#include "main_flow.hpp"

#include <ek/util/locator.hpp>

#include <ek/scenex/AudioManager.hpp>
#include <ek/scenex/particles/ParticleSystem.hpp>
#include <ek/scenex/InteractionSystem.hpp>
#include <ek/scenex/utility/destroy_delay.hpp>
#include <ek/goodies/shake_system.hpp>
#include <ek/goodies/helpers/Trail2D.hpp>
#include <ek/goodies/bubble_text.hpp>
#include <ek/goodies/popup_manager.hpp>
#include <ek/scenex/TimeLayer.hpp>
#include <ek/scenex/2d/Camera2D.hpp>
#include <ek/scenex/2d/LayoutRect.hpp>
#include <ek/scenex/2d/Button.hpp>
#include <ek/scenex/2d/Canvas.hpp>
#include <ek/scenex/2d/MovieClip.hpp>
#include <ek/scenex/base/Tween.hpp>
#include <ek/goodies/GameScreen.hpp>

namespace ek {

using namespace ecs;

void scene_pre_update(entity root, float dt) {

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
    Tween::update();
    update_shake();

    update_bubble_text(dt);
    update_popup_managers(dt);

    updateScripts();
    Button::updateAll();
    MovieClip::updateAll();
}

void scene_post_update(ecs::entity root, float dt) {
    destroy_delayed_entities(dt);
    updateWorldTransform2D(root);

    updateTrails();
    update_emitters();
    update_particles();
    Camera2D::updateQueue();
}

void scene_render(ecs::entity root) {
    Camera2D::render();
//    drawScene2D(root);
    //drawSceneGizmos(root);
}

}


