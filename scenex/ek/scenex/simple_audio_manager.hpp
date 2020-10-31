#pragma once

#include <string>
#include <ek/scenex/local_storage_var_t.hpp>
#include <ek/audio/audio.hpp>
#include <ek/util/path.hpp>

namespace ek {

class simple_audio_manager {
public:
    local_storage_var_t sound{"sound", 1};
    local_storage_var_t music{"music", 1};
    local_storage_var_t vibro{"vibro", 1};

    simple_audio_manager();

    ~simple_audio_manager();

    void play_music(const std::string& name);

    void set_music_volume(float volume) {
        music_volume_ = volume;
    }

    void play_sound(const std::string& name, float vol = 1.0f);

    void vibrate(int length) const;

    void update(float dt);

    void disable_all();

private:
    path_t base_path_;
    std::string music_;
    float music_volume_ = 1.0f;

    [[nodiscard]] std::string resolve_path(const std::string& id) const;
};

}


