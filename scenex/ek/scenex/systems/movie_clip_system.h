#pragma once

#include <ek/scenex/components/movie_t.h>
#include <ek/scenex/components/node_t.h>
#include <ek/scenex/components/display_2d.h>

namespace ek {

void update_movie_clips();

void goto_and_stop(ecs::entity e, int frame);

}


