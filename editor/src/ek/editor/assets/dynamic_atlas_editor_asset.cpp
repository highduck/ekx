#include "dynamic_atlas_editor_asset.hpp"
#include <ek/system/working_dir.hpp>
#include <ek/scenex/2d/dynamic_atlas.hpp>
#include <ek/util/assets.hpp>

namespace ek {

DynamicAtlasEditorAsset::DynamicAtlasEditorAsset(path_t path)
        : editor_asset_t{std::move(path), "dynamic_atlas_settings"} {
    reloadOnScaleFactorChanged = false;
}

void DynamicAtlasEditorAsset::read_decl_from_xml(const pugi::xml_node& node) {
}

void DynamicAtlasEditorAsset::load() {
    read_decl();

    auto scaleFactor = project->scale_factor;
    int normScaleFactor = int(ceilf(scaleFactor));
    int pageSize = std::min(1024 * normScaleFactor, 4096);

    asset_t<DynamicAtlas>{name_}.reset(new DynamicAtlas(pageSize, pageSize));
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

    working_dir_t::with(data.output, [&] {
        // save options
    });

    data.meta("dynamic_atlas", name_);
}

void DynamicAtlasEditorAsset::save() {
}

}