#include "editor_temp_atlas.hpp"
#include <ek/spritepack/sprite_packing.hpp>

namespace ek {

spritepack::atlas_t prepare_temp_atlas(const std::string& name, float scale) {
    atlas_decl_t atlas_decl;
    atlas_decl.name = name;
    atlas_decl.resolutions.resize(1);
    atlas_decl.resolutions[0] = {scale, {4096, 4096}};
    return spritepack::atlas_t{atlas_decl};
}

scenex::atlas_t* load_temp_atlas(spritepack::atlas_t& temp_atlas) {
    temp_atlas.resolutions[0].pages = spritepack::pack(temp_atlas.resolutions[0].sprites,
                                                       temp_atlas.resolutions[0].max_size);

    const auto& atlas_name = temp_atlas.name;
    auto* atlas = new scenex::atlas_t;

    int page_index = 0;
    for (const auto& page : temp_atlas.resolutions[0].pages) {
        auto& texture_asset = atlas->pages.emplace_back(atlas_name + "_page_" + std::to_string(page_index++));
        auto* texture = new texture_t();
        texture->upload(*page.image);
        texture_asset.reset(texture);

        for (auto& spr_data : page.sprites) {
            auto* sprite = new scenex::sprite_t();
            sprite->rotated = spr_data.is_rotated();
            sprite->rect = spr_data.rc;
            sprite->tex = spr_data.uv;
            sprite->texture = texture_asset;

            asset_t<scenex::sprite_t> asset_spr{spr_data.name};
            asset_spr.reset(sprite);
            atlas->sprites[spr_data.name] = asset_spr;
        }
    }

    asset_t<scenex::atlas_t>{atlas_name}.reset(atlas);

    return atlas;
}

}