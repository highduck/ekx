#include "audio_asset.hpp"
#include <ek/system/system.hpp>
#include <ek/util/Res.hpp>

namespace ek {

audio_asset_t::audio_asset_t(path_t path)
        : editor_asset_t{std::move(path), "audio"} {
}

bool check_filters(const ek::path_t& path, const Array<std::string>& filters) {
    for (const auto& filter : filters) {
        if (path.str().rfind(filter) != std::string::npos) {
            return true;
        }
    }
    return false;
}

void audio_asset_t::read_decl_from_xml(const pugi::xml_node& node) {
    Array<std::string> music_filters{};
    music_list_.clear();
    sound_list_.clear();

    for (auto music_node : node.children("music")) {
        music_filters.emplace_back(music_node.attribute("filter").as_string());
    }

    auto files = search_files("*.mp3", project->base_path / resource_path_);
    for (auto& file : files) {
        if (check_filters(file, music_filters)) {
            music_list_.push_back(file.str());
        } else {
            sound_list_.push_back(file.str());
        }
    }
}

void audio_asset_t::build(assets_build_struct_t& data) {
    read_decl();

    const path_t output_path = data.output / name_;

    make_dirs(output_path);
    for (const auto& audio : sound_list_) {
        path_t path = path_t{name_} / path_name(audio);
        copy_file(path_t{audio}, data.output / path);
        data.meta("sound", path.str());
    }
    for (const auto& audio : music_list_) {
        path_t path = path_t{name_} / path_name(audio);
        copy_file(path_t{audio}, data.output / path);
        data.meta("music", path.str());
    }
}

std::string audio_asset_t::getRelativeName(const std::string& filepath) const {
    auto name = (path_t{name_} / path_name(filepath)).str();
    // remove extension
    if (name.size() > 4) {
        name.erase(name.size() - 4, 4);
    }
    return name;
}

}