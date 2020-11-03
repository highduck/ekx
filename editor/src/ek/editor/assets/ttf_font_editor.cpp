#include "ttf_font_editor.hpp"

#include <ek/util/assets.hpp>
#include <ek/rtfont/ttf_font.hpp>
#include <ek/system/system.hpp>
#include <ek/util/logger.hpp>

namespace ek {

TTFFontEditor::TTFFontEditor(path_t path)
        : editor_asset_t{std::move(path), "freetype"} {
}

void TTFFontEditor::read_decl_from_xml(const pugi::xml_node& node) {
}

void TTFFontEditor::load() {
    read_decl();
    auto buffer = read_file(get_relative_path(resource_path_));
    if (buffer.empty()) {
        EK_ERROR("Not found or empty %s", (project->base_path / declaration_path_).c_str());
    } else {
        auto* ttfFont = new TTFFont();
        ttfFont->setScale(project->scale_factor);
        ttfFont->loadFromMemory(std::move(buffer));
        asset_t<font_t>{name_}.reset(new font_t(ttfFont));
    }
}

void TTFFontEditor::unload() {
    asset_t<TTFFont>{name_}.reset(nullptr);
}

void TTFFontEditor::gui() {
}

void TTFFontEditor::build(assets_build_struct_t& build_data) {
    read_decl();

    const auto output_path = build_data.output / name_;
    make_dirs(output_path.dir());
    auto ttfFileData = ::ek::read_file(project->base_path / resource_path_);
    ::ek::save(ttfFileData, (output_path + ".ttf").c_str());

    build_data.meta(type_name_, name_);
}

void TTFFontEditor::save() {
}

}