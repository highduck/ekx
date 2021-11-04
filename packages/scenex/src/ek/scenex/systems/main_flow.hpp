#pragma once

#include <ecxx/ecxx.hpp>

namespace ek {

void scene_pre_update(ecs::EntityApi root, float dt);
void scene_post_update(ecs::EntityApi root);
void scene_render(ecs::EntityApi root);

}


