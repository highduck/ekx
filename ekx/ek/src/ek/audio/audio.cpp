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

void AudioResource::load(const char* filepath, bool streaming) {
    buffer = auph::load(filepath, streaming ? auph::Flag_Stream : 0);
}

void AudioResource::unload() {
    if (auph::isActive(buffer.id)) {
        auph::unload(buffer);
        buffer = {};
    }
}

AudioResource::~AudioResource() {
    unload();
}

}
