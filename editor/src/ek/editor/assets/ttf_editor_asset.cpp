#include "ttf_editor_asset.hpp"

#include <ek/util/assets.hpp>
#include <ek/scenex/text/truetype_font.hpp>
#include <ek/scenex/text/font.hpp>
#include <ek/system/system.hpp>
#include <ek/util/logger.hpp>
#include <ek/app/app.hpp>

namespace ek {

TTFEditorAsset::TTFEditorAsset(path_t path)
        : editor_asset_t{std::move(path), "ttf"} {
}

void TTFEditorAsset::read_decl_from_xml(const pugi::xml_node& node) {
}

void TTFEditorAsset::load() {
    read_decl();
    auto buffer = read_file(get_relative_path(resource_path_));
    if (buffer.empty()) {
        EK_ERROR("Not found or empty %s", (project->base_path / declaration_path_).c_str());
    } else {
        auto* ttfFont = new TrueTypeFont(project->scale_factor, 48, 2048 * 2);
        ttfFont->loadFromMemory(std::move(buffer));
        asset_t<Font>{name_}.reset(new Font(ttfFont));
    }
}

void TTFEditorAsset::unload() {
    asset_t<TrueTypeFont>{name_}.reset(nullptr);
}

void TTFEditorAsset::gui() {
}

void TTFEditorAsset::build(assets_build_struct_t& build_data) {
    read_decl();

    const auto output_path = build_data.output / name_;
    make_dirs(output_path.dir());
    auto ttfFileData = ::ek::read_file(project->base_path / resource_path_);
    ::ek::save(ttfFileData, (output_path + ".ttf").c_str());

    build_data.meta(type_name_, name_);
}

void TTFEditorAsset::save() {
}

}