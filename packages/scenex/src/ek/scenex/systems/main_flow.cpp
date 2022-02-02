#include "main_flow.hpp"

#include <ekx/ng/updater.h>
#include <ekx/app/audio_manager.h>
#include <ek/scenex/particles/ParticleSystem.hpp>
#include <ek/scenex/InteractionSystem.hpp>
#include <ek/scenex/base/DestroyTimer.hpp>
#include <ek/goodies/Shake.hpp>
#include <ek/goodies/helpers/Trail2D.hpp>
#include <ek/goodies/bubble_text.hpp>
#include <ek/goodies/PopupManager.hpp>
#include <ek/scenex/2d/Camera2D.hpp>
#include <ek/scenex/2d/LayoutRect.hpp>
#include <ek/scenex/2d/Button.hpp>
#include <ek/scenex/2d/Viewport.hpp>
#include <ek/scenex/2d/MovieClip.hpp>
#include <ek/scenex/base/Tween.hpp>
#include <ek/goodies/GameScreen.hpp>
#include <ek/scenex/2d/DynamicAtlas.hpp>
#include <ek/scenex/2d/Atlas.hpp>

namespace ek {

using namespace ecs;

void scene_pre_update(EntityApi /*root*/, float dt) {
    update_interaction_system();

    update_time_layers(dt);

    audio_update(dt);

    if (g_game_screen_manager) {
        g_game_screen_manager->update();
    }

    LayoutRect::updateAll();
    Tween::updateAll();
    Shaker::updateAll();

    BubbleText::updateAll();
    popup_manager_update();

    updater_update();

    Button::updateAll();
    MovieClip::updateAll();
}

void scene_post_update(ecs::EntityApi root) {
    destroy_manager_update();

    updateWorldTransformAll2(root);

    Trail2D::updateAll();
    update_emitters();
    update_particles();
    Camera2D::updateQueue();
}

void scene_render(ecs::EntityApi root) {
    update_res_dynamic_atlas();
    update_res_atlas();

    Camera2D::render();
//    drawScene2D(root);
    //drawSceneGizmos(root);
}

}


