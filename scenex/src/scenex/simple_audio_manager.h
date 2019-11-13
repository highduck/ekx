#pragma once

#include <string>
#include <scenex/local_storage_var_t.hpp>
#include <ek/audiomini.hpp>
#include <scenex/config.h>

namespace scenex {

class simple_audio_manager {
public:
    local_storage_var_t sound{"sound", 1};
    local_storage_var_t music{"music", 1};
    local_storage_var_t vibro{"vibro", 1};

    ek::AudioMini& device;

    simple_audio_manager();

    ~simple_audio_manager();

    void play_music(const std::string& name);

    void set_music_volume(float volume) {
        music_volume_ = volume;
    }

    void play_sound(const std::string& name);

    void play_sound(const std::string& name, float vol);

    void play_sound(const std::string& name, const float2& position);

    void vibrate(int length);

    void update(float dt);

    void onMusicFft(const uint8_t* data, size_t size);

    void disable_all();

private:
    std::string base_path_;
    std::string music_;
    float music_volume_ = 1.0f;

    [[nodiscard]] std::string resolve_path(const std::string& id) const;
};

}


