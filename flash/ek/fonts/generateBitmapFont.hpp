#pragma once

#include <string>
#include <vector>

#include <ek/scenex/text/BitmapFont.hpp>
#include <ek/spritepack/sprite_data.hpp>
#include <ek/filters/filters.hpp>
#include <ek/spritepack/atlas_declaration.hpp>
#include <ek/util/path.hpp>

namespace pugi {
class xml_node;
}

namespace ek::font_lib {

struct GenBitmapFontDecl {
    uint16_t fontSize;
    std::vector<std::pair<uint32_t, uint32_t>> ranges;
    bool mirrorCase = true;
    bool useKerning = true;

    void readFromXML(const pugi::xml_node& node);

    void writeToXML(pugi::xml_node& node) const;
};

BitmapFontData generateBitmapFont(const path_t& path,
                                  const std::string& name,
                                  const GenBitmapFontDecl& decl,
                                  const FiltersDecl& filters,
                                  spritepack::atlas_t& to_atlas);

}