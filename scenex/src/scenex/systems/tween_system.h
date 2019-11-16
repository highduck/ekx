#pragma once

#include <scenex/config/ecs.h>
#include <scenex/components/tween.h>

namespace scenex {

void update_tweens();
tween_t& reset_tween(ecs::entity e);

}


