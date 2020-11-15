#include "dynamic_atlas_editor_asset.hpp"
#include <ek/system/working_dir.hpp>
#include <ek/scenex/2d/dynamic_atlas.hpp>
#include <ek/util/assets.hpp>
#include <ek/system/system.hpp>

namespace ek {

DynamicAtlasEditorAsset::DynamicAtlasEditorAsset(path_t path)
        : editor_asset_t{std::move(path), "dynamic_atlas_settings"} {
    reloadOnScaleFactorChanged = false;
}

void DynamicAtlasEditorAsset::read_decl_from_xml(const pugi::xml_node& node) {
    alphaMap = node.attribute("alphaMap").as_bool(false);
    mipmaps = node.attribute("mipmaps").as_bool(false);
}

void DynamicAtlasEditorAsset::load() {
    read_decl();

    auto scaleFactor = project->scale_factor;
    int normScaleFactor = int(ceilf(scaleFactor));
    int pageSize = std::min(1024 * normScaleFactor, 4096);

    asset_t<DynamicAtlas>{name_}.reset(new DynamicAtlas(pageSize, pageSize, alphaMap, mipmaps));
}

void DynamicAtlasEditorAsset::unload() {
    asset_t<DynamicAtlas>{name_}.reset(nullptr);
}

void DynamicAtlasEditorAsset::gui() {
    // TODO: preview
//    auto w = (float) atlas->pageWidth;
//    auto h = (float) atlas->pageHeight;
//    draw2d::state.set_empty_texture();
//    draw2d::quad(x, y, w, h, 0x77000000_argb);
//
//    draw2d::state.set_texture(atlas.texture_);
//    draw2d::state.set_texture_coords(0, 0, 1, 1);
//    draw2d::quad(x, y, w, h);
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

void DynamicAtlasEditorAsset::save() {
}

}