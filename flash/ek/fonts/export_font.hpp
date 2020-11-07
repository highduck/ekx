#pragma once

#include <string>
#include <vector>

#include <ek/scenex/text/bitmap_font.hpp>
#include <ek/spritepack/sprite_data.hpp>
#include <ek/flash/rasterizer/software_filters.hpp>
#include <ek/spritepack/atlas_declaration.hpp>
#include <ek/util/path.hpp>
#include "ft_library.hpp"
#include "font_declaration.hpp"
#include "filters_declaration.hpp"

namespace ek::font_lib {

BitmapFontData export_font(const path_t& path,
                           const std::string& name,
                           const font_decl_t& font_opts,
                           const filters_decl_t& filters_opts,
                           spritepack::atlas_t& to_atlas);

}