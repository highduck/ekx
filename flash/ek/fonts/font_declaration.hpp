#pragma once

#include <pugixml.hpp>
#include "ft_library.hpp"

namespace ek {

struct font_decl_t {
    uint16_t fontSize;
    std::vector<font_lib::code_range_t> ranges;
    bool mirrorCase = true;
    bool useKerning = true;
};

void from_xml(pugi::xml_node node, font_decl_t& font);

void to_xml(pugi::xml_node node, const font_decl_t& font);

}

