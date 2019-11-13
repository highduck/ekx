#include "audio_asset.hpp"
#include <pugixml.hpp>
#include <scenex/asset2/asset_manager.hpp>
#include <ek/fs/path.hpp>
#include <ek/logger.hpp>
#include <ek/serialize/serialize.hpp>
#include <ek/system/system.hpp>
#include <ek/audiomini.hpp>
#include <ek/locator.hpp>
#include <ek/editor/imgui/imgui.hpp>
#include <ek/editor/gui/editor_widgets.hpp>

using scenex::asset_object_t;

namespace ek {

audio_asset_t::audio_asset_t(std::string path)
        : path_{std::move(path)} {
}

void audio_asset_t::load() {
    pugi::xml_document xml;

    std::vector<std::string> music_filters;


    const auto full_path = project_->base_path / path_;
    if (xml.load_file(full_path.c_str())) {
        auto node = xml.first_child();
        name_ = node.attribute("name").as_string();
        for (auto music_node : node.children("music")) {
            music_filters.emplace_back(music_node.attribute("filter").as_string());
        }
    } else {
        EK_ERROR << "Error parse xml: " << full_path;
    }

    auto& audio = resolve<AudioMini>();
    auto files = search_files("*.mp3", project_->base_path);
    for (auto& file : files) {
        for (const auto& filter : music_filters) {
            if (file.str().rfind(filter) != std::string::npos) {
                music_list_.push_back(file.str());
                audio.create_music(file.c_str());
            } else {
                sound_list_.push_back(file.str());
                audio.create_sound(file.c_str());
            }
        }
    }
}

void audio_asset_t::unload() {
    auto& audio = resolve<AudioMini>();
    for (const auto& music : music_list_) {
//        audio.destroy_music(music.c_str());
    }
    for (const auto& sound : sound_list_) {
//        audio.destroy_sound(sound.c_str());
    }
    sound_list_.clear();
    music_list_.clear();
}

void audio_asset_t::gui() {
    auto& audio = resolve<AudioMini>();
    if (ImGui::TreeNode(this, "%s (MiniAudio)", path_.c_str())) {
        ImGui::LabelText("Name", "%s", name_.c_str());

        gui_asset_object_controls(this);

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
        ImGui::TreePop();
    }
}

void audio_asset_t::export_() {
    path_t output_path = path_t{project_->export_path} / name_;
    make_dirs(output_path);
    for (const auto& audio_file_path : sound_list_) {
        copy_file(path_t{audio_file_path}, output_path / path_name(audio_file_path));
    }
    for (const auto& audio_file_path : music_list_) {
        copy_file(path_t{audio_file_path}, output_path / path_name(audio_file_path));
    }
}

void audio_asset_t::save() {
    // TODO:
//    pugi::xml_document xml;
//    //if (xml.load_file(path_join(project_->base_path, path_).c_str())) {
//    auto node = xml.append_child("asset");
//    node.append_attribute("name").set_value(name_.c_str());
//    node.append_attribute("type").set_value("flash");
//    node.append_attribute("path").set_value(flash_path_.c_str());
//    to_xml(node.append_child("atlas"), atlas_decl_);
//
//    const auto full_path = path_join(project_->base_path, path_);
//    if (!xml.save_file(full_path.c_str())) {
//        log(log_level::error, "Error write xml file %s", full_path.c_str());
//    }
}

void audio_asset_t::export_meta(output_memory_stream& output) {
    IO io{output};

    std::string sound_type{"sound"};
    std::string music_type{"music"};

    for (auto& sound : sound_list_) {
        io(sound_type, sound);
    }

    for (auto& music : music_list_) {
        io(music_type, music);
    }
}

}