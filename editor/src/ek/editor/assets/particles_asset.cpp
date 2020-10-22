#include "particles_asset.hpp"
#include <ek/system/system.hpp>

namespace ek {

particles_asset_t::particles_asset_t(path_t path)
        : editor_asset_t{std::move(path), "particles"} {
}

void particles_asset_t::read_decl_from_xml(const pugi::xml_node& node) {

}

void particles_asset_t::load() {
    read_decl();

    asset_t<particle_decl>{name_}.reset(&decl_);
}

void particles_asset_t::unload() {
    asset_t<particle_decl>{name_}.reset(nullptr);
}

void particles_asset_t::gui() {

}

void particles_asset_t::build(assets_build_struct_t& data) {
    read_decl();

//    auto output_path = project->export_path / name_;
//
//    make_dirs(output_path.dir());
//
//    output_memory_stream out{100};
//    IO io{out};
//    io(decl_);
//    ::ek::save(out, output_path + ".particles");

    data.meta("particles", name_);
}

void particles_asset_t::save() {
    // TODO:
}

}