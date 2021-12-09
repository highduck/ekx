#pragma once

#include <ek/scenex/StorageVariable.hpp>
#include <ek/audio/audio.hpp>
#include <ek/ds/String.hpp>
#include <ek/util/Path.hpp>
#include <ek/util/Type.hpp>
#include <ek/math/Vec.hpp>

namespace ek {

class AudioManager {
public:
    StorageVariable sound{"sound", 1};
    StorageVariable music{"music", 1};
    StorageVariable vibro{"vibro", 1};

    AudioManager();

    ~AudioManager();

    void play_music(const char* name);

    void setMusicParams(float volume = 1.0f, float pitch = 1.0f);
    void play_sound(const char* name, float vol = 1.0f, float pitch = 1.0f) const;
    void play_sound_at(const char* name, const Vec2f& position, float vol = 1.0f, float pitch = 1.0f) const;

    void vibrate(int length) const;

    void update(float dt);

    void disable_all();

private:
    String music_;
    auph::Voice musicVoice_{};
    float musicVolume_ = 1.0f;
    float musicPitch_ = 1.0f;
};

EK_DECLARE_TYPE(AudioManager);

}


