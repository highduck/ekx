#pragma once

#include <vector>
#include "sprite_data.hpp"

namespace ek::spritepack {

void export_atlas(atlas_t& atlas);
std::string dump_resolution_json(atlas_resolution_t& resolution);

}


