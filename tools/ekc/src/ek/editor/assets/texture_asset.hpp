#pragma once

#include "editor_asset.hpp"
#include <ek/ds/Array.hpp>
#include <ek/util/Path.hpp>

namespace ek {

class TextureAsset : public editor_asset_t {
public:
    inline static const char* type_name = "texture";
    
    explicit TextureAsset(path_t path);

    void read_decl_from_xml(const pugi::xml_node& node) override;

    void build(assets_build_struct_t& data) override;

private:
    std::string texture_type_;
    Array<std::string> images_{};
};

}