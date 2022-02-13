#include "audio_manager.h"

void play_music(string_hash_t name) {
    res_id next_music = name ? R_AUDIO(name) : 0;
    if (g_audio.music_ != next_music) {
        if (auph_is_active(g_audio.musicVoice_.id)) {
            auph_stop(g_audio.musicVoice_.id);
            g_audio.musicVoice_ = {};
        }

        if (next_music) {
            auph_buffer buffer = REF_RESOLVE(res_audio, next_music);
            if (buffer.id) {
                if (auph_is_buffer_loaded(buffer) && !auph_is_active(g_audio.musicVoice_.id)) {
                    g_audio.musicVoice_ = auph_play_f(buffer,
                                                      g_audio.musicVolume_, 0.0f, g_audio.musicPitch_,
                                                      true, false,
                                                      AUPH_BUS_MUSIC);
                }
            }
        }
        g_audio.music_ = next_music;
    }
}

void play_sound(string_hash_t name, float vol, float pitch) {
    if ((g_audio.prefs & AUDIO_PREF_SOUND) && name) {
        const res_id id = R_AUDIO(name);
        if (id) {
            auph_buffer snd = REF_RESOLVE(res_audio, id);
            if (snd.id) {
                auph_play_f(snd, vol, 0.0f, pitch, false, false, AUPH_BUS_SOUND);
            }
        }
    }
}

void play_sound_at(string_hash_t name, const vec2_t position, float volume, float pitch) {
    if (g_audio.prefs & AUDIO_PREF_SOUND) {
        auto relVolume = volume;
        //auto spatialPanning = -1 .. 1;
        play_sound(name, relVolume, pitch);
    }
}

void vibrate(int length) {
    if ((g_audio.prefs & AUDIO_PREF_VIBRO) && length > 0) {
        auph_vibrate(length);
    }
}

void update_audio_manager() {
    const uint32_t prefs = g_audio.prefs;
    auph_set_gain(AUPH_BUS_MUSIC.id, (prefs & AUDIO_PREF_MUSIC) ? 1.0f : 0.0f);
    auph_set_gain(AUPH_BUS_SOUND.id, (prefs & AUDIO_PREF_SOUND) ? 1.0f : 0.0f);

    R(auph_buffer) music_res = g_audio.music_;
    if (music_res) {
        const auph_voice music_voice = g_audio.musicVoice_;
        const auph_buffer music_buffer = REF_RESOLVE(res_audio, music_res);
        if (music_buffer.id && auph_is_buffer_loaded(music_buffer) && !auph_is_active(music_voice.id)) {
            const float vol = g_audio.musicVolume_;
            const float rate = g_audio.musicPitch_;
            g_audio.musicVoice_ = auph_play_f(music_buffer, vol, 0.0f, rate, true, false, AUPH_BUS_MUSIC);
        }
    }
}

void audio_disable_all() {
    // do not save, it's like dev option
    g_audio.prefs = 0;
}

void set_music_params(float volume, float pitch) {
    g_audio.musicVolume_ = volume;
    g_audio.musicPitch_ = pitch;
    if (auph_is_active(g_audio.musicVoice_.id)) {
        auph_set_gain(g_audio.musicVoice_.id, g_audio.musicVolume_);
        auph_set_rate(g_audio.musicVoice_, pitch);
    }
}

audio_manager_t g_audio;

void init_audio_manager(void) {
    if (ek_ls_get_i("sound", 1)) g_audio.prefs |= AUDIO_PREF_SOUND;
    if (ek_ls_get_i("music", 1)) g_audio.prefs |= AUDIO_PREF_MUSIC;
    if (ek_ls_get_i("vibro", 1)) g_audio.prefs |= AUDIO_PREF_VIBRO;
    g_audio.musicVolume_ = 1.0f;
    g_audio.musicPitch_ = 1.0f;
}

bool audio_toggle_pref(uint32_t pref) {
    g_audio.prefs ^= pref;
    const bool enabled = g_audio.prefs & pref;
    const char* pref_name = NULL;
    switch (pref) {
        case AUDIO_PREF_SOUND:
            pref_name = "sound";
            break;
        case AUDIO_PREF_MUSIC:
            pref_name = "music";
            break;
        case AUDIO_PREF_VIBRO:
            pref_name = "vibro";
            break;
    }

    if (pref_name) {
        ek_ls_set_i(pref_name, enabled ? 1 : 0);
    }
    return enabled;
}
