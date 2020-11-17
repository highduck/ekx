#include "AudioManager.hpp"
#include "scene_system.hpp"

#include <ek/app/device.hpp>
#include <utility>

namespace ek {

AudioManager::AudioManager() = default;
AudioManager::~AudioManager() = default;

void AudioManager::play_music(const std::string& name) {
    if (name != music_) {
        Res<audio::Music> musicAsset{music_};
        if (musicAsset) {
            musicAsset->stop();
        }
        musicAsset.setID(name);
        if (musicAsset) {
            auto volume = music.enabled() ? music_volume_ : 0.0f;
            musicAsset->setVolume(volume);
            musicAsset->play();
        }
        music_ = name;
    }
}

void AudioManager::play_sound(const std::string& name, float vol) const {
    if (sound.enabled()) {
        Res<audio::Sound> soundAsset{name};
        if (soundAsset) {
            soundAsset->play(vol);
        }
    }
}

void AudioManager::play_sound_at(const std::string& name, const float2& position, float volume) const {
    if (sound.enabled()) {
        auto relVolume = volume;
        //auto spatialPanning = -1 .. 1;
        play_sound(name, relVolume);
    }
}

void AudioManager::vibrate(int length) const {
    if (vibro.enabled() && length > 0) {
        ek::vibrate(length);
    }
}

void AudioManager::update(float) {
    if(!music_.empty()) {
        Res<audio::Music> musicAsset{music_};
        if (musicAsset) {
            auto volume = music.enabled() ? music_volume_ : 0.0f;
            musicAsset->setVolume(volume);
            musicAsset->update();
        }
    }
}

void AudioManager::disable_all() {
    sound.enabled(false);
    music.enabled(false);
    vibro.enabled(false);
}

}