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

void scene_pre_update(entity_t root, float dt) {
    update_interaction_system();

    update_time_layers(dt);

    update_audio_manager();

    if (g_game_screen_manager) {
        g_game_screen_manager->update();
    }

    LayoutRect::updateAll();
    update_tweens();
    Shaker::updateAll();

    BubbleText::updateAll();
    update_popup_manager();

    Button::updateAll();
    MovieClip::updateAll();
}

void scene_post_update(entity_t root) {
    update_destroy_queue();

    update_world_transform_2d(root);

    update_trail2d();
    update_emitters();
    update_particles();
    update_camera2d_queue();
}

void scene_render(entity_t root) {
    update_res_dynamic_atlas();
    update_res_atlas();
    render_camera2d_queue();
}

}


