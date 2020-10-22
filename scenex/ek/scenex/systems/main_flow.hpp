#pragma once

#include <ecxx/ecxx.hpp>

namespace ek {

void scene_pre_update(ecs::entity root, float dt);
void scene_post_update(ecs::entity root, float dt);
void scene_render(ecs::entity root);

}


