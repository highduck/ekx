#include "AudioManager.hpp"

namespace ek {

AudioManager::AudioManager() = default;

AudioManager::~AudioManager() = default;

void AudioManager::play_music(string_hash_t name) {
    res_id next_music = name ? R_AUDIO(name) : 0;
    if (music_ != next_music) {
        if (auph_is_active(musicVoice_.id)) {
            auph_stop(musicVoice_.id);
            musicVoice_ = {};
        }

        if (next_music) {
            auph_buffer buffer = REF_RESOLVE(res_audio, next_music);
            if (buffer.id) {
                if (auph_is_buffer_loaded(buffer) && !auph_is_active(musicVoice_.id)) {
                    musicVoice_ = auph_play_f(buffer,
                                              musicVolume_, 0.0f, musicPitch_,
                                              true, false,
                                              AUPH_BUS_MUSIC);
                }
            }
        }
        music_ = next_music;
    }
}

void AudioManager::play_sound(string_hash_t name, float vol, float pitch) const {
    if (sound.enabled() && name) {
        const res_id id = R_AUDIO(name);
        if (id) {
            auph_buffer snd = REF_RESOLVE(res_audio, id);
            if (snd.id) {
                auph_play_f(snd, vol, 0.0f, pitch, false, false, AUPH_BUS_SOUND);
            }
        }
    }
}

void AudioManager::play_sound_at(string_hash_t name, const vec2_t position, float volume, float pitch) const {
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

    if (music_) {
        const auph_buffer buff = REF_RESOLVE(res_audio, music_);
        if (buff.id && auph_is_buffer_loaded(buff) && !auph_is_active(musicVoice_.id)) {
            musicVoice_ = auph_play_f(buff, musicVolume_, 0.0f, musicPitch_, true, false, AUPH_BUS_MUSIC);
        }
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