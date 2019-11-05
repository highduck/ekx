#include "main_flow.h"

#include <platform/window.hpp>
#include <ek/locator.hpp>

#include <scenex/components/transform_2d.h>
#include <scenex/simple_audio_manager.h>
#include <scenex/particles/particle_system.h>
#include <scenex/goodies/shake_system.h>
#include <scenex/interactive_manager.h>
#include <scenex/utility/destroy_delay.h>
#include <scenex/scene_system.h>

#include "game_time.h"
#include "layout_system.h"
#include "tween_system.h"
#include "canvas_system.h"
#include "button_system.h"
#include "movie_clip_system.h"

namespace scenex {

using namespace ecs;
using namespace ek;

void smoothly_advance_time(entity root, float dt) {
//    static float steps[3] = {0.0f, 0.0f, 0.0f};
//    steps[2] = steps[1];
//    steps[1] = steps[0];
//    steps[0] = dt;
//    auto avg = (steps[0] + steps[1] + steps[2]) / 3.0f;
    update_time(root, dt);
//    update_time(w, root, 1.0f / 60.0f);
}

void scene_pre_update(entity root, float dt) {
    const auto screen_size = g_window.back_buffer_size;
    ecs::get<transform_2d>(root).rect = rect_f{
            0.0f,
            0.0f,
            static_cast<float>(screen_size.width),
            static_cast<float>(screen_size.height)
    };

    resolve<simple_audio_manager>().update(dt);
    smoothly_advance_time(root, dt);

    update_canvas();
    update_layout();
    update_tweens();
    update_emitters();
    update_particles();
    update_shake();

}

void scene_post_update(ecs::entity root, float dt) {
    resolve<interactive_manager>().update();
    update_nodes(root, dt);
    update_buttons(dt);
    update_movie_clips();
    destroy_delayed_entities(dt);
}

void scene_render(ecs::entity root) {
    draw_node(root);
}

}


