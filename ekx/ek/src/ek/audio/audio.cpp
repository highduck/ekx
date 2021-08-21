#include "audio.hpp"

#include <ek/assert.hpp>
#include <ek/debug.hpp>

// auph implementation
#define AUPH_MP3

#include <auph/auph_impl.hpp>

#if defined(__ANDROID__)
#include <ek/android.hpp>
#endif

namespace ek::audio {

struct AudioSystem {
    int locks = 0;
    bool initialized = false;
};

static AudioSystem audioSystem{};

void initialize() {
    EK_TRACE << "audio initialize";
    EK_ASSERT(!audioSystem.initialized);
#if defined(__ANDROID__)
    auto* env = ek::android::get_jni_env();
    auto activity = ek::android::get_activity();
    auto assets = ek::android::assetManagerRef();
    auph::setAndroidActivity(env, activity, assets);
#endif
    auph::init();
    audioSystem.initialized = true;
}

void shutdown() {
    EK_TRACE << "audio shutdown";
    EK_ASSERT(audioSystem.initialized);
    auph::shutdown();
    audioSystem.initialized = false;
}

void muteDeviceBegin() {
    EK_ASSERT(audioSystem.locks >= 0);
    if (audioSystem.locks == 0) {
        auph::setGain(auph::hMixer, 0.0f);
    }
    ++audioSystem.locks;
}

void muteDeviceEnd() {
    --audioSystem.locks;
    EK_ASSERT(audioSystem.locks >= 0);
    if (audioSystem.locks == 0) {
        auph::setGain(auph::hMixer, 1.0f);
    }
}

#if !defined(__APPLE__) && !defined(__ANDROID__)

void vibrate(int duration_millis) {

}

#endif

/** Sound **/

void Sound::load(const char* path) {
    dataSourceFilePath = path;
    buffer = auph::load(path, 0);
    if (!buffer.id) {
        EK_WARN("Error load sound: %s", dataSourceFilePath.c_str());
        unload();
    }
}

void Sound::unload() {
    if (auph::isActive(buffer.id)) {
        auph::unload(buffer);
        buffer = {};
    }
}

Sound::~Sound() {
    unload();
}

void Sound::play(float volume, float pitch) {
    if (auph::isActive(buffer.id) && volume > 0.0f) {
        auph::play(buffer, volume, 0.0f, pitch, false, false, auph::Bus_Sound);
    }
}

Sound::Sound(const char* path) {
    load(path);
}

/** Music **/

void Music::load(const char* path) {
    dataSourceFilePath = path;
    buffer = auph::load(path, auph::Flag_Stream);
    if (!buffer.id) {
        EK_WARN("Error load music stream: %s", dataSourceFilePath.c_str());
        unload();
    }
}

void Music::unload() {
    if (auph::isActive(buffer.id)) {
        auph::unload(buffer);
        buffer = {};
    }
    voice = {};
}

Music::~Music() {
    unload();
}

void Music::play(float volume, float pitch) {
    if (!auph::isActive(voice.id)) {
        voice = auph::play(buffer, volume, 0.0f, pitch, true, false, auph::Bus_Music);
    }
}

void Music::stop() {
    if (auph::isActive(voice.id)) {
        auph::stop(voice.id);
        voice = {};
    }
}

void Music::setVolume(float volume) {
    auph::setGain(voice.id, volume);
}

float Music::getVolume() const {
    return auph::getGain(voice.id);
}

void Music::setPitch(float pitch) {
    auph::setRate(voice, pitch);
}

float Music::getPitch() const {
    return auph::getRate(voice);
}

Music::Music(const char* path) {
    load(path);
}

}
