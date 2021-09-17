#pragma once

#include <vector>
#include <pugixml.hpp>

#include <ek/builders/BitmapFontBuilder.hpp>
#include "atlas.hpp"

namespace ek {

void exportBitmapFont(const char* xmlPath) {
    pugi::xml_document xml;
    if (!xml.load_file(xmlPath)) {
        EK_ERROR_F("error parse xml %s", xmlPath);
        return;
    }
    auto node = xml.first_child();
    const auto* name = node.attribute("name").as_string();
    const auto* path = node.attribute("path").as_string();
    const auto* outputSprites = node.attribute("outputSprites").as_string();
    const auto* outputFont = node.attribute("outputFont").as_string();

    BuildBitmapFontSettings font{};
    SpriteFilterList filters{};
    MultiResAtlasSettings atlasSettings{};
    font.readFromXML(node.child("font"));
    filters.readFromXML(node.child("filters"));
    atlasSettings.readFromXML(node.child("atlas"));

    MultiResAtlasData atlas{atlasSettings};

    EK_DEBUG("Export Bitmap Font asset");

    auto fontData = buildBitmapFont(path_t{path}, name, font, filters, atlas);
    ek::output_memory_stream out{100};
    IO io{out};
    io(fontData);
    ::ek::save(out, outputFont);

    saveImages(atlas, outputSprites);
}

}