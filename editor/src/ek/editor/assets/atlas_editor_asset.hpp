#pragma once


#include "editor_asset.hpp"
#include "editor_temp_atlas.hpp"
#include <ek/spritepack/atlas_declaration.hpp>

namespace ek {

class AtlasEditorAsset : public editor_asset_t {
public:
    inline static const char* type_name = "atlas_builder";

    explicit AtlasEditorAsset(path_t path);

    void read_decl_from_xml(const pugi::xml_node& node) override;

    void beforeLoad() override;

    void afterLoad() override;

    void unload() override;

    void gui() override;

    void beforeBuild(assets_build_struct_t& data) override;

    void afterBuild(assets_build_struct_t& data) override;

    void save() override;

private:
    atlas_decl_t decl_;
};


}

