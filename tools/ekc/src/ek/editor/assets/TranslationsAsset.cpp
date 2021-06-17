#include <ek/system/system.hpp>
#include <ek/serialize/serialize.hpp>
#include <ek/serialize/stl/String.hpp>
#include "TranslationsAsset.hpp"

namespace ek {


TranslationsAsset::TranslationsAsset(path_t path) :
        editor_asset_t{std::move(path), "strings"} {
}

void TranslationsAsset::read_decl_from_xml(const pugi::xml_node& node) {
    languages.clear();
    auto files = search_files("*.po", project->base_path / resource_path_);
    for (auto& file : files) {
        languages[file.basename().withoutExt()] = project->base_path / resource_path_ / file.basename();
    }
}

void TranslationsAsset::build(assets_build_struct_t& build_data) {
    read_decl();

    const auto output_path = build_data.output / name_;
    make_dirs(output_path);

    Array<std::string> langs{};
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

}