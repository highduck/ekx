#pragma once

#include "editor_asset.hpp"

namespace ek {

class DynamicAtlasEditorAsset : public editor_asset_t {
public:
    inline static const char* type_name = "dynamic_atlas_settings";

    explicit DynamicAtlasEditorAsset(path_t path);

    void read_decl_from_xml(const pugi::xml_node& node) override;

    void load() override;

    void unload() override;

    void gui() override;

    void build(assets_build_struct_t& data) override;

    void save() override;

private:

};


}

