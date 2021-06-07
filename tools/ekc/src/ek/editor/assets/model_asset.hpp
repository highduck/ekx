#pragma once

#include "editor_asset.hpp"

namespace ek {

class ModelAsset : public editor_asset_t {
public:
    inline static const char* type_name = "model";
    explicit ModelAsset(path_t path);

    void read_decl_from_xml(const pugi::xml_node& node) override;

    void build(assets_build_struct_t& data) override;

};

}