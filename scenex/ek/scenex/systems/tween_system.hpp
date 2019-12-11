#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/scenex/components/tween.hpp>

namespace ek {

void update_tweens();
tween_t& reset_tween(ecs::entity e);

}


