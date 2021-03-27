#pragma once

#include <string>
#include <vector>

#include <ek/scenex/text/BitmapFont.hpp>
#include <ek/builders/MultiResAtlas.hpp>
#include <ek/builders/SpriteFilters.hpp>
#include <ek/util/path.hpp>

namespace pugi {
class xml_node;
}

namespace ek {

struct BuildBitmapFontSettings {
    uint16_t fontSize;
    std::vector<std::pair<uint32_t, uint32_t>> ranges;
    bool mirrorCase = true;
    bool useKerning = true;

    void readFromXML(const pugi::xml_node& node);

    void writeToXML(pugi::xml_node& node) const;
};

BitmapFontData buildBitmapFont(const path_t& path,
                               const std::string& name,
                               const BuildBitmapFontSettings& decl,
                               const SpriteFilterList& filters,
                               MultiResAtlasData& toAtlas);

}