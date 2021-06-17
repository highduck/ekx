#include "editor_project.hpp"
#include "editor_asset.hpp"
#include "TextureAsset.hpp"
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
#include <ek/debug.hpp>
#include <functional>

namespace ek {

Array<path_t> search_asset_files(const path_t& path) {
    working_dir_t in{path};
    return search_files("*.asset.xml", path_t{""});
}

void editor_project_t::populate() {
    clear();
    for (const auto& path : search_asset_files(base_path)) {
        add_file(path);
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
        delete asset;
    }
    assets.clear();
}

editor_project_t::editor_project_t() {
    register_asset_factory<TextureAsset>();
    register_asset_factory<BitmapFontEditorAsset>();
    register_asset_factory<flash_asset_t>();
    register_asset_factory<ModelAsset>();
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
    Array<editor_asset_t*> assetsToBuild{};
    // read declaration for all populated assets,
    // and add to queue assets that should be packed (dev / platform)
    // platform filter is not implemented yet
    for (auto* asset : assets) {
        asset->read_decl();
        // skip dev files for build without editor
        if (devMode || !asset->isDev()) {
            assetsToBuild.push_back(asset);
        }
    }
    make_dirs(output);
    output_memory_stream out{100};
    assets_build_struct_t build_data{output, &out};
    // then all queued assets should pass 3 steps: before build, build, after build
    for (auto asset : assetsToBuild) asset->beforeBuild(build_data);
    for (auto asset : assetsToBuild) asset->build(build_data);
    for (auto asset : assetsToBuild) asset->afterBuild(build_data);

    build_data.meta("", "");
    ::ek::save(out, output / "pack_meta");
}

}