#pragma once

#include <scenex/2d/atlas.hpp>
#include <ek/spritepack/sprite_data.hpp>

namespace ek {

spritepack::atlas_t prepare_temp_atlas(const std::string& name, float scale);

scenex::atlas_t* load_temp_atlas(spritepack::atlas_t& temp_atlas);

}