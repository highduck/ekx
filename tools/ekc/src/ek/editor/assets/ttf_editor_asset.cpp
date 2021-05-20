#include "ttf_editor_asset.hpp"

#include <ek/util/Res.hpp>
#include <ek/system/system.hpp>

#include <ek/serialize/serialize.hpp>
#include <ek/serialize/stl/String.hpp>

namespace ek {

TTFEditorAsset::TTFEditorAsset(path_t path) :
        editor_asset_t{std::move(path), "ttf"} {
}

void TTFEditorAsset::read_decl_from_xml(const pugi::xml_node& node) {
    glyphCache = node.attribute("glyphCache").as_string("default_glyph_cache");
    baseFontSize = node.attribute("fontSize").as_float(48);
}

void TTFEditorAsset::build(assets_build_struct_t& build_data) {
    read_decl();

    const auto output_path = build_data.output / name_;
    make_dirs(output_path.dir());

    auto ttfFileData = ::ek::read_file(project->base_path / resource_path_);
    ek::save(ttfFileData, output_path + ".ttf");

    // save TTF options
    output_memory_stream out{100};
    IO io{out};
    io(baseFontSize, glyphCache);
    ek::save(out, output_path + ".ttf_settings");

    build_data.meta(type_name_, name_);
}

}