#pragma once

#include "sprite_data.hpp"

namespace ek::spritepack {

std::vector<atlas_page_t> pack(std::vector<sprite_t> sprites, int2 max_size);

std::vector<sprite_t> unpack(const std::vector<atlas_page_t>& pages);

std::vector<atlas_page_t> repack(const std::vector<atlas_page_t>& pages, int2 max_size);

}

