#pragma once

#include <ek/spritepack/sprite_data.hpp>
#include <ek/imaging/filters.hpp>

namespace ek::flash {

void apply(const std::vector<filter_data_t>& filters, spritepack::sprite_t& sprite, float scale);

}


