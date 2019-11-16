#pragma once

#include "scenex/components/movie_t.h"
#include "scenex/config.h"
#include "scenex/components/node_t.h"
#include "scenex/components/display_2d.h"

namespace scenex {

void update_movie_clips();

void goto_and_stop(ecs::entity e, int frame);

}


