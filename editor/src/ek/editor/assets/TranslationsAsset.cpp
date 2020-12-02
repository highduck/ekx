#include <ek/system/system.hpp>
#include <ek/serialize/serialize.hpp>
#include <ek/Localization.hpp>
#include "TranslationsAsset.hpp"

namespace ek {


TranslationsAsset::TranslationsAsset(path_t path)
        : editor_asset_t{std::move(path), "strings"} {
}

void TranslationsAsset::read_decl_from_xml(const pugi::xml_node& node) {
    languages.clear();
    auto files = search_files("*.po", project->base_path / resource_path_);
    for (auto& file : files) {
        languages[file.basename().withoutExt()] = project->base_path / resource_path_ / file.basename();
    }
}

void TranslationsAsset::load() {
    read_decl();

    for (auto& it : languages) {
        const auto& lang = it.first;
        execute("msgfmt --output-file=" + lang + ".mo " + it.second.str());
        Localization::instance.load(lang, std::move(read_file(path_t{lang + ".mo"})));
    }
//    auto buffer = read_file(get_relative_path(resource_path_));
//    if (buffer.empty()) {
//        EK_ERROR("Not found or empty %s", (project->base_path / declaration_path_).c_str());
//    } else {
//        auto* ttfFont = new TrueTypeFont(project->scale_factor, baseFontSize, glyphCache);
//        ttfFont->loadFromMemory(std::move(buffer));
//        Res<Font>{name_}.reset(new Font(ttfFont));
//    }
}

void TranslationsAsset::unload() {
//    Res<TrueTypeFont>{name_}.reset(nullptr);
}

void TranslationsAsset::gui() {
}

void TranslationsAsset::build(assets_build_struct_t& build_data) {
    read_decl();

    const auto output_path = build_data.output / name_;
    make_dirs(output_path);

    std::vector<std::string> langs;
    for (auto& it : languages) {
        auto& lang = it.first;
        langs.push_back(lang);
        execute("msgfmt --output-file=" + (output_path / lang).str() + ".mo " + it.second.str());
    }

    // save TTF options
    output_memory_stream out{100};
    IO io{out};
    io(langs);
    ek::save(out, output_path / "strings.bin");

    build_data.meta(type_name_, name_);
}

void TranslationsAsset::save() {
}


}