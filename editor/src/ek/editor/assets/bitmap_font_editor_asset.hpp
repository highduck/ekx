#pragma once

#include "editor_asset.hpp"
#include <ek/spritepack/atlas_declaration.hpp>
#include <ek/fonts/generateBitmapFont.hpp>
#include <ek/filters/filters.hpp>

namespace ek {

class BitmapFontEditorAsset : public editor_asset_t {
public:
    inline static const char* type_name = "bmfont";

    explicit BitmapFontEditorAsset(path_t path);

    void read_decl_from_xml(const pugi::xml_node& node) override;

    void load() override;

    void unload() override;

    void gui() override;

    void build(assets_build_struct_t& data) override;

    void save() override;

private:
    std::string atlasTarget_;
    font_lib::GenBitmapFontDecl font_;
    FiltersDecl filters_;
};

}