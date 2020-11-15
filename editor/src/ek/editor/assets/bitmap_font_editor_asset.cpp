#include "bitmap_font_editor_asset.hpp"

#include <ek/editor/gui/gui.hpp>
#include <ek/util/logger.hpp>
#include <ek/system/working_dir.hpp>
#include <ek/spritepack/export_atlas.hpp>
#include <ek/editor/imgui/imgui.hpp>
#include <ek/scenex/2d/sprite.hpp>
#include <ek/system/system.hpp>
#include <ek/fonts/export_font.hpp>
#include <ek/scenex/text/font.hpp>
#include <utility>
#include <memory>

namespace ek {

BitmapFontEditorAsset::BitmapFontEditorAsset(path_t path)
        : editor_asset_t{std::move(path), "bmfont"} {
    reloadOnScaleFactorChanged = true;
}

void BitmapFontEditorAsset::read_decl_from_xml(const pugi::xml_node& node) {
    atlasTarget_ = node.attribute("atlas").value();

    font_decl_ = {};
    from_xml(node.child("font"), font_decl_);

    filters_decl_ = {};
    from_xml(node.child("filters"), filters_decl_);
}

void BitmapFontEditorAsset::load() {
    read_decl();

    asset_t<spritepack::atlas_t> atlasBuild{atlasTarget_};
    auto font_data = font_lib::export_font(project->base_path / resource_path_,
                                           name_,
                                           font_decl_,
                                           filters_decl_,
                                           atlasBuild.mutableRef());

    auto* bmFont = new BitmapFont();
    bmFont->load(font_data);
    asset_t<Font>{name_}.reset(new Font(bmFont));
}

void BitmapFontEditorAsset::unload() {
    asset_t<Font>{name_}.reset(nullptr);
}

void BitmapFontEditorAsset::gui() {
    asset_t<Font> font{name_};
    gui_font_view(font.get());
}

void BitmapFontEditorAsset::build(assets_build_struct_t& data) {
    read_decl();

    asset_t<spritepack::atlas_t> atlasBuild{atlasTarget_};
    auto font_data = font_lib::export_font(project->base_path / resource_path_,
                                           name_,
                                           font_decl_,
                                           filters_decl_,
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

void BitmapFontEditorAsset::save() {
    // TODO:
//    pugi::xml_document xml;
//    //if (xml.load_file(path_join(project->base_path, path_).c_str())) {
//    auto node = xml.append_child("asset");
//    node.append_attribute("name").set_value(name_.c_str());
//    node.append_attribute("type").set_value("flash");
//    node.append_attribute("path").set_value(flash_path_.c_str());
//    to_xml(node.append_child("atlas"), atlas_decl_);
//
//    const auto full_path = path_join(project->base_path, path_);
//    if (!xml.save_file(full_path.c_str())) {
//        log(log_level::error, "Error write xml file %s", full_path.c_str());
//    }
}

}