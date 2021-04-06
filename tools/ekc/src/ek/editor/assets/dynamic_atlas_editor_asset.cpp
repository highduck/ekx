#include "dynamic_atlas_editor_asset.hpp"
#include <ek/system/working_dir.hpp>
#include <ek/util/Res.hpp>
#include <ek/system/system.hpp>

namespace ek {

DynamicAtlasEditorAsset::DynamicAtlasEditorAsset(path_t path)
        : editor_asset_t{std::move(path), "dynamic_atlas_settings"} {
}

void DynamicAtlasEditorAsset::read_decl_from_xml(const pugi::xml_node& node) {
    alphaMap = node.attribute("alphaMap").as_bool(false);
    mipmaps = node.attribute("mipmaps").as_bool(false);
}

void DynamicAtlasEditorAsset::build(assets_build_struct_t& data) {
    read_decl();

    const auto output_path = data.output / name_;
    make_dirs(output_path.dir());

    // save TTF options
    output_memory_stream out{100};
    IO io{out};
    io(alphaMap, mipmaps);
    ek::save(out, output_path + ".dynamic_atlas");

    data.meta("dynamic_atlas", name_);
}

}