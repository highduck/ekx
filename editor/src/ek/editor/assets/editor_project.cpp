#include "editor_project.hpp"
#include "editor_asset.hpp"
#include "texture_asset.hpp"
#include "bitmap_font_editor_asset.hpp"
#include "flash_asset.hpp"
#include "model_asset.hpp"
#include "audio_asset.hpp"
#include "ttf_editor_asset.hpp"
#include "MultiResAtlasEditorAsset.hpp"
#include "dynamic_atlas_editor_asset.hpp"
#include "TranslationsAsset.hpp"

#include <ek/system/working_dir.hpp>
#include <ek/system/system.hpp>
#include <ek/serialize/serialize.hpp>
#include <ek/util/logger.hpp>
#include <functional>

namespace ek {


void editor_project_t::update_scale_factor(float scaleFactor, bool notifyAssets) {
    const int uid = math::clamp(static_cast<int>(std::ceil(scaleFactor)), 1, 4);
    scale_factor = math::clamp(scaleFactor, 1.0f, 4.0f);
    if (scale_uid != uid) {
        scale_uid = uid;
        // notify only if scale index changed
        if (notifyAssets) {
            load_all(true);
        }
    }
}

std::vector<path_t> search_asset_files(const path_t& path) {
    working_dir_t in{path};
    return search_files("*.asset.xml", path_t{""});
}

void editor_project_t::populate(bool auto_load) {
    clear();
    for (const auto& path : search_asset_files(base_path)) {
        add_file(path);
    }
    if (auto_load) {
        load_all();
    }
}

struct AssetInfoHeader {
    std::string type;
    bool dev = false;
};

AssetInfoHeader openAssetInfoHeader(const path_t& path) {
    AssetInfoHeader result{};
    pugi::xml_document doc{};
    if (doc.load_file(path.c_str())) {
        result.type = doc.first_child().attribute("type").as_string();
        result.dev = doc.first_child().attribute("dev").as_bool(false);
    } else {
        EK_ERROR("XML parsing error %s", path.c_str());
    }
    return result;
}

void editor_project_t::add_file(const path_t& path) {
    const auto header = openAssetInfoHeader(base_path / path);
    auto factory_method = type_factory[header.type];
    if (factory_method) {
        auto* asset = factory_method(path);
        assert(asset);
        asset->project = this;
        assets.push_back(asset);
    } else {
        EK_ERROR("Editor asset type '%s' not found", header.type.c_str());
    }
}

void editor_project_t::clear() {
    for (auto asset : assets) {
        asset->unload();
        delete asset;
    }
    assets.clear();
}

void editor_project_t::load_all(bool reloadAfterScaleChanged) {
    std::vector<editor_asset_t*> assetsToLoad;
    if (reloadAfterScaleChanged) {
        for (auto* asset : assets) {
            if (asset->reloadOnScaleFactorChanged) {
                assetsToLoad.push_back(asset);
            }
        }
    } else {
        assetsToLoad = assets;
    }
    for (auto& asset : assetsToLoad) asset->beforeLoad();
    for (auto& asset : assetsToLoad) asset->load();
    for (auto& asset : assetsToLoad) asset->afterLoad();
}

void editor_project_t::unload_all() {
    for (auto asset : assets) {
        asset->unload();
    }
}

editor_project_t::editor_project_t() {
    register_asset_factory<texture_asset_t>();
    register_asset_factory<BitmapFontEditorAsset>();
    register_asset_factory<flash_asset_t>();
    register_asset_factory<model_asset_t>();
    register_asset_factory<audio_asset_t>();
    register_asset_factory<TTFEditorAsset>();
    register_asset_factory<MultiResAtlasEditorAsset>();
    register_asset_factory<DynamicAtlasEditorAsset>();
    register_asset_factory<TranslationsAsset>();
}

editor_project_t::~editor_project_t() {
    clear();
}

void editor_project_t::build(const path_t& output) const {
    std::vector<editor_asset_t*> assetsToBuild;
    for (auto* asset : assets) {
        // skip dev files for build without editor
        if (!asset->isDev()) {
            assetsToBuild.push_back(asset);
        }
    }
    make_dirs(output);
    output_memory_stream out{100};
    assets_build_struct_t build_data{output, &out};
    for (auto asset : assetsToBuild) asset->beforeBuild(build_data);
    for (auto asset : assetsToBuild) asset->build(build_data);
    for (auto asset : assetsToBuild) asset->afterBuild(build_data);

    build_data.meta("", "");
    ::ek::save(out, output / "pack_meta");
}

}