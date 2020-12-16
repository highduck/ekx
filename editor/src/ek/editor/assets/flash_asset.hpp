#pragma once

#include "editor_asset.hpp"
#include <ek/builders/MultiResAtlas.hpp>
#include <memory>

namespace ek {

class flash_asset_t : public editor_asset_t {
public:
    inline static const char* type_name = "flash";

    explicit flash_asset_t(path_t path);

    void read_decl_from_xml(const pugi::xml_node& node) override;

    void load() override;

    void unload() override;

    void gui() override;

    void build(assets_build_struct_t& data) override;

    void save() override;
private:
    std::string atlasTarget_;
};

}

