#include "simple_audio_manager.h"

#include <ek/locator.hpp>

namespace scenex {

simple_audio_manager::simple_audio_manager()
        : device{ek::resolve<ek::AudioMini>()},
          base_path_{"assets/sfx/"} {

}

simple_audio_manager::~simple_audio_manager() = default;

void simple_audio_manager::play_music(const std::string& name) {
    if (name.empty() && !music_.empty()) {
        device.play_music(resolve_path(music_).c_str(), 0.0);
    }
    music_ = name;
}

void simple_audio_manager::play_sound(const std::string& name) {
    if (sound.enabled() && !name.empty()) {
        device.play_sound(resolve_path(name).c_str(), 1.0f, 0.0f);
    }
}

void simple_audio_manager::play_sound(const std::string& name, float vol) {
    if (sound.enabled() && !name.empty()) {
        device.play_sound(resolve_path(name).c_str(), vol, 0.0);
    }
}

void simple_audio_manager::play_sound(const std::string& name, const float2& position) {
    (void)position;
    play_sound(name);
}

void simple_audio_manager::vibrate(int length) {
    if (vibro.enabled() && length > 0) {
        device.vibrate(length);
    }
}

void simple_audio_manager::update(float) {
    if (!music_.empty()) {
        device.play_music(resolve_path(music_).c_str(), music.enabled() ? music_volume_ : 0.0f);
    }
}

void simple_audio_manager::onMusicFft(const uint8_t*, size_t) {
//    if (fft == null) {
//        fft = new U8s(data.length);
//    }
//    fft.copyFrom(data);
}

void simple_audio_manager::disable_all() {
    sound.enabled(false);
    music.enabled(false);
    vibro.enabled(false);
}

std::string simple_audio_manager::resolve_path(const std::string& id) const {
    return base_path_ + id + ".mp3";
}


}