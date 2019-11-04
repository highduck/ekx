#pragma once

#include <string>
#include <vector>

#include <scenex/2d/font.hpp>
#include <ek/spritepack/sprite_data.hpp>
#include <ek/flash/rasterizer/software_filters.h>
#include <ek/spritepack/atlas_declaration.hpp>
#include <ek/fs/path.hpp>
#include "ft_library.h"
#include "font_declaration.hpp"
#include "filters_declaration.hpp"

namespace ek::font_lib {

scenex::font_data_t export_font(const path_t& path,
                                const std::string& name,
                                const font_decl_t& font_opts,
                                const filters_decl_t& filters_opts,
                                spritepack::atlas_t& to_atlas);

}