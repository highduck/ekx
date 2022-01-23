#pragma once

#include <ek/scenex/StorageVariable.hpp>
#include <ek/audio.h>
#include <ek/math.h>
#include <ek/util/Type.hpp>

namespace ek {

class AudioManager {
public:
    StorageVariable sound{"sound", 1};
    StorageVariable music{"music", 1};
    StorageVariable vibro{"vibro", 1};

    AudioManager();

    ~AudioManager();

    void play_music(string_hash_t name);

    void setMusicParams(float volume = 1.0f, float pitch = 1.0f);
    void play_sound(string_hash_t name, float vol = 1.0f, float pitch = 1.0f) const;
    void play_sound_at(string_hash_t name, vec2_t position, float vol = 1.0f, float pitch = 1.0f) const;

    void vibrate(int length) const;

    void update(float dt);

    void disable_all();

private:
    R(auph_buffer) music_ = 0;
    auph_voice musicVoice_{};
    float musicVolume_ = 1.0f;
    float musicPitch_ = 1.0f;
};

}

extern ek::AudioManager* g_audio;
void init_audio_manager(void);
