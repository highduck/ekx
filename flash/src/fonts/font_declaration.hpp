#pragma once

#include <pugixml.hpp>
#include "ft_library.h"

namespace ek {

struct font_decl_t {
    std::vector<uint16_t> sizes;
    std::vector<font_lib::code_range_t> ranges;
    bool mirror_case = true;
    bool use_kerning = true;
};

void from_xml(pugi::xml_node node, font_decl_t& font);

void to_xml(pugi::xml_node node, const font_decl_t& font);

}

