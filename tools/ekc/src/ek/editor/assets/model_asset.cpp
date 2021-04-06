#include "model_asset.hpp"

#include <ek/scenex/data/Model3D.hpp>
#include <ek/system/system.hpp>
#include <ek/util/Res.hpp>
#include <ek/editor/obj/obj_loader.hpp>

namespace ek {

model_asset_t::model_asset_t(path_t path)
        : editor_asset_t{std::move(path), "model"} {
}

void model_asset_t::read_decl_from_xml(const pugi::xml_node& node) {

}

void model_asset_t::build(assets_build_struct_t& build_data) {
    read_decl();

    const auto output_path = build_data.output / name_;
    Model3D data{
            load_obj(read_file(get_relative_path(resource_path_)))
    };

    make_dirs(output_path.dir());

    output_memory_stream out{100};
    IO io{out};
    io(data);
    ::ek::save(out, output_path + ".model");

    build_data.meta(type_name_, name_);
}

}