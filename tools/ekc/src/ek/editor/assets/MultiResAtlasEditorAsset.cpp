#include "MultiResAtlasEditorAsset.hpp"

#include <ek/system/working_dir.hpp>
#include <ek/util/Res.hpp>
#include <memory>
#include <utility>

namespace ek {

MultiResAtlasEditorAsset::MultiResAtlasEditorAsset(path_t path)
        : editor_asset_t{std::move(path), "atlas_builder"} {
}

void MultiResAtlasEditorAsset::read_decl_from_xml(const pugi::xml_node& node) {
    settings_ = {};
    settings_.readFromXML(node);
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

}