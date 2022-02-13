#pragma once

#include <ecx/ecx.hpp>

namespace ek {

void scene_pre_update(entity_t root, float dt);
void scene_post_update(entity_t root);
void scene_render(entity_t root);

}


