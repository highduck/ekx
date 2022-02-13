#pragma once

#include <ek/scenex/StorageVariable.hpp>
#include <ek/audio.h>
#include <ek/math.h>

enum {
    AUDIO_PREF_SOUND = 1,
    AUDIO_PREF_MUSIC = 2,
    AUDIO_PREF_VIBRO = 4,
};

typedef struct audio_manager_t {
    uint32_t prefs;
    R(auph_buffer) music_;
    auph_voice musicVoice_;
    float musicVolume_;
    float musicPitch_;
} audio_manager_t;

void play_music(string_hash_t name);

void set_music_params(float volume = 1.0f, float pitch = 1.0f);

void play_sound(string_hash_t name, float vol = 1.0f, float pitch = 1.0f);

void play_sound_at(string_hash_t name, vec2_t position, float vol = 1.0f, float pitch = 1.0f);

void vibrate(int length);

void update_audio_manager();

void audio_disable_all();

extern audio_manager_t g_audio;

void init_audio_manager(void);

bool audio_toggle_pref(uint32_t pref);
