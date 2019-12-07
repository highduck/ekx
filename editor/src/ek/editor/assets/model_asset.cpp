#include "model_asset.hpp"

#include <scenex/3d/static_mesh.hpp>
#include <ek/system/system.hpp>
#include <ek/util/logger.hpp>
#include <ek/util/assets.hpp>
#include <graphics/vertex_decl.hpp>
#include <scenex/data/model_data.hpp>
#include <ek/editor/obj/obj_loader.hpp>

namespace ek {

model_asset_t::model_asset_t(path_t path)
        : editor_asset_t{std::move(path), "model"} {
}

void model_asset_t::read_decl_from_xml(const pugi::xml_node& node) {

}

void model_asset_t::load() {
    read_decl();

    auto buffer = read_file(get_relative_path(resource_path_));
    if (buffer.empty()) {
        EK_ERROR("Not found or empty %s", (project->base_path / declaration_path_).c_str());
    } else {
        asset_t<scenex::static_mesh_t>{name_}.reset(
                new scenex::static_mesh_t(
                        load_obj(buffer)
                )
        );
    }
}

void model_asset_t::unload() {
    asset_t<scenex::static_mesh_t>{name_}.reset(nullptr);
}

void model_asset_t::gui() {
}

void model_asset_t::build(assets_build_struct_t& build_data) {
    read_decl();

    const auto output_path = build_data.output / name_;
    scenex::model_data_t data{
            load_obj(read_file(get_relative_path(resource_path_)))
    };

    make_dirs(output_path.dir());

    output_memory_stream out{100};
    IO io{out};
    io(data);
    ::ek::save(out, output_path + ".model");

    build_data.meta(type_name_, name_);
}

void model_asset_t::save() {
    // TODO:
}

}