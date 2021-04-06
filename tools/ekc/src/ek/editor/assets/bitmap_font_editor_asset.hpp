#pragma once

#include "editor_asset.hpp"
#include <ek/builders/MultiResAtlas.hpp>
#include <ek/builders/BitmapFontBuilder.hpp>
#include <ek/builders/SpriteFilters.hpp>

namespace ek {

class BitmapFontEditorAsset : public editor_asset_t {
public:
    inline static const char* type_name = "bmfont";

    explicit BitmapFontEditorAsset(path_t path);

    void read_decl_from_xml(const pugi::xml_node& node) override;

    void build(assets_build_struct_t& data) override;

private:
    std::string atlasTarget_;
    BuildBitmapFontSettings font_;
    SpriteFilterList filters_;
};

}