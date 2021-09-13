#include "AudioManager.hpp"

#include <ek/app/app.hpp>
#include <ek/util/Res.hpp>

namespace ek {

AudioManager::AudioManager() = default;

AudioManager::~AudioManager() = default;

void AudioManager::play_music(const std::string& name) {
    if (name != music_) {
        if (auph::isActive(musicVoice_.id)) {
            auph::stop(musicVoice_.id);
            musicVoice_ = {};
        }

        Res<audio::AudioResource> audioRes{music_};
        audioRes.setID(name);
        if (audioRes) {
            if (auph::isBufferLoaded(audioRes->buffer) && !auph::isActive(musicVoice_.id)) {
                musicVoice_ = auph::play(audioRes->buffer,
                                         musicVolume_, 0.0f, musicPitch_,
                                         true, false,
                                         auph::Bus_Music);
            }
        }
        music_ = name;
    }
}

void AudioManager::play_sound(const std::string& name, float vol, float pitch) const {
    if (sound.enabled()) {
        Res<audio::AudioResource> snd{name};
        if (snd) {
            auph::play(snd->buffer, vol, 0.0f, pitch);
        }
    }
}

void AudioManager::play_sound_at(const std::string& name, const float2& position, float volume, float pitch) const {
    if (sound.enabled()) {
        auto relVolume = volume;
        //auto spatialPanning = -1 .. 1;
        play_sound(name, relVolume, pitch);
    }
}

void AudioManager::vibrate(int length) const {
    if (vibro.enabled() && length > 0) {
        app::vibrate(length);
    }
}

void AudioManager::update(float) {
    auph::setGain(auph::Bus_Music.id, music.enabled() ? 1.0f : 0.0f);
    auph::setGain(auph::Bus_Sound.id, sound.enabled() ? 1.0f : 0.0f);

    if (music_.empty()) {
        return;
    }
    Res<audio::AudioResource> audioRes{music_};
    if (audioRes && auph::isBufferLoaded(audioRes->buffer) && !auph::isActive(musicVoice_.id)) {
        musicVoice_ = auph::play(audioRes->buffer, musicVolume_, 0.0f, musicPitch_, true, false, auph::Bus_Music);
    }
}

void AudioManager::disable_all() {
    sound.enabled(false);
    music.enabled(false);
    vibro.enabled(false);
}

void AudioManager::setMusicParams(float volume, float pitch) {
    musicVolume_ = volume;
    musicPitch_ = pitch;
    if (auph::isActive(musicVoice_.id)) {
        auph::setGain(musicVoice_.id, musicVolume_);
        auph::setRate(musicVoice_, pitch);
    }
}

}