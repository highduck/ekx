#pragma once

#include <string>
#include <ek/scenex/local_storage_var_t.hpp>
#include <ek/audio/audio.hpp>
#include <ek/util/path.hpp>

namespace ek {

class AudioManager {
public:
    local_storage_var_t sound{"sound", 1};
    local_storage_var_t music{"music", 1};
    local_storage_var_t vibro{"vibro", 1};

    AudioManager();

    ~AudioManager();

    void play_music(const std::string& name);

    void set_music_volume(float volume) {
        music_volume_ = volume;
    }

    void play_sound(const std::string& name, float vol = 1.0f, bool multi = false) const;
    void play_sound_at(const std::string& name, const float2& position, float vol = 1.0f) const;

    void vibrate(int length) const;

    void update(float dt);

    void disable_all();

private:
    std::string music_;
    float music_volume_ = 1.0f;
};

}


