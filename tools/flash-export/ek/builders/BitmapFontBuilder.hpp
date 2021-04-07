#pragma once

#include <string>
#include <ek/ds/Array.hpp>
#include <ek/scenex/data/BMFont.hpp>
#include <ek/builders/MultiResAtlas.hpp>
#include <ek/builders/SpriteFilters.hpp>
#include <ek/util/path.hpp>

namespace pugi {
class xml_node;
}

namespace ek {

struct CodepointPair {
    uint32_t a;
    uint32_t b;
};

struct BuildBitmapFontSettings {
    uint16_t fontSize;
    Array<CodepointPair> ranges{};
    bool mirrorCase = true;
    bool useKerning = true;

    void readFromXML(const pugi::xml_node& node);

    void writeToXML(pugi::xml_node& node) const;
};

BMFont buildBitmapFont(const path_t& path,
                               const std::string& name,
                               const BuildBitmapFontSettings& decl,
                               const SpriteFilterList& filters,
                               MultiResAtlasData& toAtlas);

}