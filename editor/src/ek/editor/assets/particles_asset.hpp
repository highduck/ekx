#pragma once

#include "editor_asset.hpp"
#include <scenex/particles/particle_decl.h>

namespace ek {

// TODO:

class particles_asset_t : public editor_asset_t {
public:
    inline static const char* type_name = "particles";
    explicit particles_asset_t(path_t path);

    void read_decl_from_xml(const pugi::xml_node& node) override;

    void load() override;

    void unload() override;

    void gui() override;

    void build(assets_build_struct_t& data) override;

    void save() override;

private:
    scenex::particle_decl decl_;
};


}

