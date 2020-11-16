#pragma once

#include <ek/scenex/components/movie.hpp>
#include <ek/scenex/components/node.hpp>
#include <ek/scenex/2d/Display2D.hpp>

namespace ek {

void update_movie_clips();

void goto_and_stop(ecs::entity e, float frame);

}


