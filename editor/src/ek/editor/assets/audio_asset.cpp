#include "audio_asset.hpp"
#include <ek/serialize/serialize.hpp>
#include <ek/system/system.hpp>
#include <ek/audiomini.hpp>
#include <ek/locator.hpp>
#include <ek/editor/imgui/imgui.hpp>

namespace ek {

audio_asset_t::audio_asset_t(path_t path)
        : editor_asset_t{std::move(path), "audio"} {
}

bool check_filters(const ek::path_t& path, const std::vector<std::string>& filters) {
    for (const auto& filter : filters) {
        if (path.str().rfind(filter) != std::string::npos) {
            return true;
        }
    }
    return false;
}

void audio_asset_t::read_decl_from_xml(const pugi::xml_node& node) {
    std::vector<std::string> music_filters;
    music_list_.clear();
    sound_list_.clear();

    for (auto music_node : node.children("music")) {
        music_filters.emplace_back(music_node.attribute("filter").as_string());
    }

    auto files = search_files("*.mp3", project->base_path);
    for (auto& file : files) {
        if (check_filters(file, music_filters)) {
            music_list_.push_back(file.str());
        } else {
            sound_list_.push_back(file.str());
        }
    }
}

void audio_asset_t::load() {
    read_decl();

    auto& audio = resolve<AudioMini>();
    for (auto& m: music_list_) {
        audio.create_music(m.c_str());
    }
    for (auto& m: sound_list_) {
        audio.create_sound(m.c_str());
    }
}

void audio_asset_t::unload() {
//    auto& audio = resolve<AudioMini>();
//    for (const auto& music : music_list_) {
//        audio.destroy_music(music.c_str());
//    }
//    for (const auto& sound : sound_list_) {
//        audio.destroy_sound(sound.c_str());
//    }
    sound_list_.clear();
    music_list_.clear();
}

void audio_asset_t::gui() {
    auto& audio = resolve<AudioMini>();

    for (const auto& music : music_list_) {
        ImGui::PushID(music.c_str());
        ImGui::LabelText("Music", "%s", music.c_str());
        if (ImGui::Button("Play Music")) {
            audio.play_music(music.c_str(), 1.0f);
        }
        if (ImGui::Button("Stop Music")) {
            audio.play_music(music.c_str(), 0.0f);
        }
        ImGui::PopID();
    }
    for (const auto& sound : sound_list_) {
        ImGui::PushID(sound.c_str());
        ImGui::LabelText("Sound", "%s", sound.c_str());
        if (ImGui::Button("Play Sound")) {
            audio.play_sound(sound.c_str(), 1.0f, 0.0f);
        }
        ImGui::PopID();
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

void audio_asset_t::save() {
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