#include "ttf_editor_asset.hpp"

#include <ek/util/assets.hpp>
#include <ek/scenex/text/TrueTypeFont.hpp>
#include <ek/scenex/text/Font.hpp>
#include <ek/system/system.hpp>
#include <ek/util/logger.hpp>

namespace ek {

TTFEditorAsset::TTFEditorAsset(path_t path)
        : editor_asset_t{std::move(path), "ttf"} {
}

void TTFEditorAsset::read_decl_from_xml(const pugi::xml_node& node) {
    glyphCache = node.attribute("glyphCache").as_string("default_glyph_cache");
    baseFontSize = node.attribute("fontSize").as_float(48);
}

void TTFEditorAsset::load() {
    read_decl();
    auto buffer = read_file(get_relative_path(resource_path_));
    if (buffer.empty()) {
        EK_ERROR("Not found or empty %s", (project->base_path / declaration_path_).c_str());
    } else {
        auto* ttfFont = new TrueTypeFont(project->scale_factor, baseFontSize, glyphCache);
        ttfFont->loadFromMemory(std::move(buffer));
        Res<Font>{name_}.reset(new Font(ttfFont));
    }
}

void TTFEditorAsset::unload() {
    Res<TrueTypeFont>{name_}.reset(nullptr);
}

void TTFEditorAsset::gui() {
}

void TTFEditorAsset::build(assets_build_struct_t& build_data) {
    read_decl();

    const auto output_path = build_data.output / name_;
    make_dirs(output_path.dir());

    auto ttfFileData = ::ek::read_file(project->base_path / resource_path_);
    ek::save(ttfFileData, output_path + ".ttf");

    // save TTF options
    output_memory_stream out{100};
    IO io{out};
    io(baseFontSize, glyphCache);
    ek::save(out, output_path + ".ttf_settings");

    build_data.meta(type_name_, name_);
}

void TTFEditorAsset::save() {
}

}