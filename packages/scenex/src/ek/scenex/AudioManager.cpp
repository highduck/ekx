#include "AudioManager.hpp"

#include <ek/app.h>
#include <ek/util/Res.hpp>

namespace ek {

AudioManager::AudioManager() = default;

AudioManager::~AudioManager() = default;

void AudioManager::play_music(const char* name) {
    if (music_ != name) {
        if (auph_is_active(musicVoice_.id)) {
            auph_stop(musicVoice_.id);
            musicVoice_ = {};
        }

        Res<AudioResource> audioRes{name};
        if (audioRes) {
            if (auph_is_buffer_loaded(audioRes->buffer) && !auph_is_active(musicVoice_.id)) {
                musicVoice_ = auph_play_f(audioRes->buffer,
                                         musicVolume_, 0.0f, musicPitch_,
                                         true, false,
                                         AUPH_BUS_MUSIC);
            }
        }
        music_ = name;
    }
}

void AudioManager::play_sound(const char* name, float vol, float pitch) const {
    if (sound.enabled()) {
        Res<AudioResource> snd{name};
        if (snd) {
            auph_play_f(snd->buffer, vol, 0.0f, pitch, false, false, AUPH_BUS_SOUND);
        }
    }
}

void AudioManager::play_sound_at(const char* name, const vec2_t position, float volume, float pitch) const {
    if (sound.enabled()) {
        auto relVolume = volume;
        //auto spatialPanning = -1 .. 1;
        play_sound(name, relVolume, pitch);
    }
}

void AudioManager::vibrate(int length) const {
    if (vibro.enabled() && length > 0) {
        auph_vibrate(length);
    }
}

void AudioManager::update(float) {
    auph_set_gain(AUPH_BUS_MUSIC.id, music.enabled() ? 1.0f : 0.0f);
    auph_set_gain(AUPH_BUS_SOUND.id, sound.enabled() ? 1.0f : 0.0f);

    if (music_.empty()) {
        return;
    }
    Res<AudioResource> audioRes{music_.c_str()};
    if (audioRes && auph_is_buffer_loaded(audioRes->buffer) && !auph_is_active(musicVoice_.id)) {
        musicVoice_ = auph_play_f(audioRes->buffer, musicVolume_, 0.0f, musicPitch_, true, false, AUPH_BUS_MUSIC);
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
    if (auph_is_active(musicVoice_.id)) {
        auph_set_gain(musicVoice_.id, musicVolume_);
        auph_set_rate(musicVoice_, pitch);
    }
}

}