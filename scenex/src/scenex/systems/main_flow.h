#pragma once

#include <scenex/config/ecs.h>

namespace scenex {

void scene_pre_update(ecs::entity root, float dt);
void scene_post_update(ecs::entity root, float dt);
void scene_render(ecs::entity root);

}


