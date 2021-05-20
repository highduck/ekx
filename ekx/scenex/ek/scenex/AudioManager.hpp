#pragma once

#include <string>
#include <ek/scenex/local_storage_var_t.hpp>
#include <ek/audio/audio.hpp>
#include <ek/util/Path.hpp>
#include <ek/util/Type.hpp>

namespace ek {

class AudioManager {
public:
    local_storage_var_t sound{"sound", 1};
    local_storage_var_t music{"music", 1};
    local_storage_var_t vibro{"vibro", 1};

    AudioManager();

    ~AudioManager();

    void play_music(const std::string& name);

    void setMusicParams(float volume = 1.0f, float pitch = 1.0f);
    void play_sound(const std::string& name, float vol = 1.0f, float pitch = 1.0f) const;
    void play_sound_at(const std::string& name, const float2& position, float vol = 1.0f, float pitch = 1.0f) const;

    void vibrate(int length) const;

    void update(float dt);

    void disable_all();

private:
    std::string music_;
    float musicVolume_ = 1.0f;
    float musicPitch_ = 1.0f;
};

EK_DECLARE_TYPE(AudioManager);

}


