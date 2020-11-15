#include "atlas_editor_asset.hpp"

#include <ek/editor/gui/editor_widgets.hpp>
#include <ek/editor/assets/editor_temp_atlas.hpp>
#include <ek/util/logger.hpp>
#include <ek/system/working_dir.hpp>
#include <ek/spritepack/export_atlas.hpp>
#include <ek/editor/imgui/imgui.hpp>
#include <ek/scenex/2d/atlas.hpp>
#include <ek/scenex/2d/sprite.hpp>
#include <ek/system/system.hpp>
#include <ek/fonts/export_font.hpp>
#include <ek/scenex/text/font.hpp>
#include <memory>
#include <utility>
#include <ek/spritepack/sprite_packing.hpp>
#include <ek/graphics/texture.hpp>

namespace ek {

AtlasEditorAsset::AtlasEditorAsset(path_t path)
        : editor_asset_t{std::move(path), "atlas_builder"} {
    reloadOnScaleFactorChanged = true;
}

void AtlasEditorAsset::read_decl_from_xml(const pugi::xml_node& node) {
    decl_ = {};
    from_xml(node, decl_);
}

void AtlasEditorAsset::beforeLoad() {
    read_decl();

    auto scaleFactor = project->scale_factor;
    atlas_decl_t tempDecl;
    tempDecl.name = name_;
    tempDecl.resolutions.resize(1);
    tempDecl.resolutions[0] = {scaleFactor, {4096, 4096}};

    asset_t<spritepack::atlas_t>{name_}.reset(new spritepack::atlas_t(tempDecl));

    // destroy prev atlas
    asset_t<atlas_t>{name_}.reset(nullptr);
}

void AtlasEditorAsset::afterLoad() {
    editor_asset_t::afterLoad();

    // pack new atlas
    asset_t<spritepack::atlas_t> atlasBuild{name_};
    atlasBuild->resolutions[0].pages = spritepack::pack(atlasBuild->resolutions[0].sprites,
                                                        atlasBuild->resolutions[0].max_size);

    auto* atlas = new atlas_t;
    int page_index = 0;
    for (const auto& page : atlasBuild->resolutions[0].pages) {
        auto& texture_asset = atlas->pages.emplace_back(name_ + "_page_" + std::to_string(page_index++));
        auto* texture = new graphics::texture_t();
        texture->upload(*page.image);
        texture_asset.reset(texture);

        for (auto& spr_data : page.sprites) {
            auto* sprite = new sprite_t();
            sprite->rotated = spr_data.is_rotated();
            sprite->rect = spr_data.rc;
            sprite->tex = spr_data.uv;
            sprite->texture = texture_asset;

            asset_t<sprite_t> asset_spr{spr_data.name};
            asset_spr.reset(sprite);
            atlas->sprites[spr_data.name] = asset_spr;
        }
    }

    asset_t<atlas_t>{name_}.reset(atlas);
    atlasBuild.reset(nullptr);
}

void AtlasEditorAsset::unload() {
    asset_t<atlas_t>{name_}.reset(nullptr);
}

void AtlasEditorAsset::gui() {
    asset_t<atlas_t> atlas{name_};
    gui_atlas_view(atlas.get());
}

void AtlasEditorAsset::beforeBuild(assets_build_struct_t& data) {
    read_decl();

    asset_t<spritepack::atlas_t>{name_}.reset(new spritepack::atlas_t(decl_));
}

void AtlasEditorAsset::afterBuild(assets_build_struct_t& data) {
    asset_t<spritepack::atlas_t> atlasBuild{name_};
    working_dir_t::with(data.output, [&] {
        spritepack::export_atlas(atlasBuild.mutableRef());
    });

    atlasBuild.reset(nullptr);

    data.meta("atlas", name_);
}

void AtlasEditorAsset::save() {
}

}