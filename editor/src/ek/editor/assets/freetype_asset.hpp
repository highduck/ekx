#pragma once

#include "editor_asset.hpp"
#include <ek/spritepack/atlas_declaration.hpp>
#include <fonts/filters_declaration.hpp>
#include <fonts/font_declaration.hpp>

namespace ek {

class freetype_asset_t : public editor_asset_t {
public:
    inline static const char* type_name = "freetype";

    explicit freetype_asset_t(path_t path);

    void read_decl_from_xml(const pugi::xml_node& node) override;

    void load() override;

    void unload() override;

    void gui() override;

    void build(assets_build_struct_t& data) override;

    void save() override;

private:
    font_decl_t font_decl_;
    atlas_decl_t atlas_decl_;
    filters_decl_t filters_decl_;
};

}