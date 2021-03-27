#pragma once

#include "editor_asset.hpp"
#include <ek/builders/MultiResAtlas.hpp>

namespace ek {

class MultiResAtlasEditorAsset : public editor_asset_t {
public:
    inline static const char* type_name = "atlas_builder";

    explicit MultiResAtlasEditorAsset(path_t path);

    void read_decl_from_xml(const pugi::xml_node& node) override;

    void beforeLoad() override;

    void afterLoad() override;

    void unload() override;

    void gui() override;

    void beforeBuild(assets_build_struct_t& data) override;

    void afterBuild(assets_build_struct_t& data) override;

    void save() override;

private:
    MultiResAtlasSettings settings_;
};


}

