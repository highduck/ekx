#pragma once

#include <ek/scenex/config/ecs.h>

namespace ek {

void scene_pre_update(ecs::entity root, float dt);
void scene_post_update(ecs::entity root, float dt);
void scene_render(ecs::entity root);

}


