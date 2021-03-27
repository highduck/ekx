#include "MultiResAtlasEditorAsset.hpp"

#include <ek/system/working_dir.hpp>
#include <ek/editor/imgui/imgui.hpp>
#include <ek/scenex/2d/Atlas.hpp>
#include <ek/scenex/2d/Sprite.hpp>
#include <memory>
#include <utility>
#include <ek/graphics/Helpers.hpp>
#include <ek/editor/gui/gui.hpp>

namespace ek {

MultiResAtlasEditorAsset::MultiResAtlasEditorAsset(path_t path)
        : editor_asset_t{std::move(path), "atlas_builder"} {
    reloadOnScaleFactorChanged = true;
}

void MultiResAtlasEditorAsset::read_decl_from_xml(const pugi::xml_node& node) {
    settings_ = {};
    settings_.readFromXML(node);
}

void MultiResAtlasEditorAsset::beforeLoad() {
    read_decl();

    auto scaleFactor = project->scale_factor;
    MultiResAtlasSettings tempDecl;
    tempDecl.name = name_;
    tempDecl.resolutions.resize(1);
    tempDecl.resolutions[0] = {scaleFactor, {4096, 4096}};

    Res<MultiResAtlasData>{name_}.reset(new MultiResAtlasData(tempDecl));

    // destroy prev atlas
    Res<Atlas>{name_}.reset(nullptr);
}

void MultiResAtlasEditorAsset::afterLoad() {
    editor_asset_t::afterLoad();

    // pack new atlas
    Res<MultiResAtlasData> atlasBuild{name_};
    atlasBuild->resolutions[0].pages = packSprites(atlasBuild->resolutions[0].sprites,
                                                        atlasBuild->resolutions[0].max_size);

    auto* atlas = new Atlas;
    int page_index = 0;
    for (const auto& page : atlasBuild->resolutions[0].pages) {
        auto& texture_asset = atlas->pages.emplace_back(name_ + "_page_" + std::to_string(page_index++));
        auto* texture = graphics::createTexture(*page.image);
        texture_asset.reset(texture);

        for (auto& spr_data : page.sprites) {
            auto* sprite = new Sprite();
            sprite->rotated = spr_data.is_rotated();
            sprite->rect = spr_data.rc;
            sprite->tex = spr_data.uv;
            sprite->texture = texture_asset;

            Res<Sprite> asset_spr{spr_data.name};
            asset_spr.reset(sprite);
            atlas->sprites[spr_data.name] = asset_spr;
        }
    }

    Res<Atlas>{name_}.reset(atlas);
    atlasBuild.reset(nullptr);
}

void MultiResAtlasEditorAsset::unload() {
    Res<Atlas>{name_}.reset(nullptr);
}

void MultiResAtlasEditorAsset::gui() {
    Res<Atlas> atlas{name_};
    gui_atlas_view(atlas.get());
}

void MultiResAtlasEditorAsset::beforeBuild(assets_build_struct_t& data) {
    read_decl();

    Res<MultiResAtlasData>{name_}.reset(new MultiResAtlasData(settings_));
}

void MultiResAtlasEditorAsset::afterBuild(assets_build_struct_t& data) {
    Res<MultiResAtlasData> atlasBuild{name_};
    working_dir_t::with(data.output, [&] {
        atlasBuild.mutableRef().packAndSaveMultiThreaded();
    });

    atlasBuild.reset(nullptr);

    data.meta("atlas", name_);
}

void MultiResAtlasEditorAsset::save() {
}

}