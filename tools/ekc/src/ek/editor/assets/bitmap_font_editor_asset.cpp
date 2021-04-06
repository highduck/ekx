#include "bitmap_font_editor_asset.hpp"

#include <ek/util/logger.hpp>
#include <ek/util/Res.hpp>
#include <ek/system/working_dir.hpp>
#include <ek/system/system.hpp>
#include <ek/builders/BitmapFontBuilder.hpp>
#include <utility>
#include <memory>

namespace ek {

BitmapFontEditorAsset::BitmapFontEditorAsset(path_t path) :
        editor_asset_t{std::move(path), "bmfont"} {
}

void BitmapFontEditorAsset::read_decl_from_xml(const pugi::xml_node& node) {
    atlasTarget_ = node.attribute("atlas").value();

    font_ = {};
    filters_ = {};

    font_.readFromXML(node.child("font"));
    filters_.readFromXML(node.child("filters"));
}

void BitmapFontEditorAsset::build(assets_build_struct_t& data) {
    read_decl();

    Res<MultiResAtlasData> atlasBuild{atlasTarget_};
    auto font_data = buildBitmapFont(project->base_path / resource_path_,
                                     name_,
                                     font_,
                                     filters_,
                                     atlasBuild.mutableRef());

    working_dir_t::with(data.output, [&] {
        EK_DEBUG << "Export Freetype asset: " << current_working_directory();
        ek::output_memory_stream out{100};
        IO io{out};
        io(font_data);
        ::ek::save(out, name_ + ".font");
    });

    data.meta("bmfont", name_);
}

}