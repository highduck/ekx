#include "main_flow.hpp"

#include <ek/util/locator.hpp>

#include <ek/scenex/AudioManager.hpp>
#include <ek/scenex/particles/particle_system.hpp>
#include <ek/scenex/interactive_manager.hpp>
#include <ek/scenex/utility/destroy_delay.hpp>
#include <ek/scenex/scene_system.hpp>
#include <ek/goodies/shake_system.hpp>
#include <ek/goodies/helpers/Trail2D.hpp>
#include <ek/goodies/bubble_text.hpp>
#include <ek/goodies/popup_manager.hpp>
#include <ek/scenex/game_time.hpp>
#include <ek/scenex/2d/Camera2D.hpp>

#include "layout_system.hpp"
#include "tween_system.hpp"
#include "canvas_system.hpp"
#include "button_system.hpp"
#include "movie_clip_system.hpp"

namespace ek {

using namespace ecs;

void scene_pre_update(entity root, float dt) {
    TimeLayer::updateTimers(dt);

    resolve<AudioManager>().update(dt);

    update_canvas();
    update_layout();
    update_tweens();
    update_shake();

    update_bubble_text(dt);
    update_popup_managers(dt);

    resolve<interactive_manager>().update();
    updateScripts();
    update_buttons(dt);
    update_movie_clips();
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


