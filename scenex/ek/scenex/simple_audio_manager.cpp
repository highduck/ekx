#include "simple_audio_manager.hpp"

#include <ek/util/locator.hpp>
#include <ek/app/device.hpp>
#include <utility>

namespace ek {

simple_audio_manager::simple_audio_manager()
        : base_path_{"assets/sfx"} {

}

simple_audio_manager::~simple_audio_manager() = default;

void simple_audio_manager::play_music(const std::string& name) {
    if (name.empty() && !music_.empty()) {
        audio::play_music(resolve_path(music_).c_str(), 0.0);
    }
    music_ = name;
}

void simple_audio_manager::play_sound(const std::string& name, float vol) {
    if (sound.enabled() && !name.empty()) {
        audio::play_sound(resolve_path(name).c_str(), vol);
    }
}

void simple_audio_manager::vibrate(int length) const {
    if (vibro.enabled() && length > 0) {
        ek::vibrate(length);
    }
}

void simple_audio_manager::update(float) {
    if (!music_.empty()) {
        audio::play_music(resolve_path(music_).c_str(), music.enabled() ? music_volume_ : 0.0f);
    }
}

void simple_audio_manager::disable_all() {
    sound.enabled(false);
    music.enabled(false);
    vibro.enabled(false);
}

std::string simple_audio_manager::resolve_path(const std::string& id) const {
    return ((base_path_ / id) + ".mp3").str();
}


}