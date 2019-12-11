#pragma once

#include <ek/scenex/config/ecs.h>
#include <ek/scenex/components/tween.h>

namespace ek {

void update_tweens();
tween_t& reset_tween(ecs::entity e);

}


