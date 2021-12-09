#include "audio.hpp"

#include <ek/log.h>
#include <ek/assert.h>

// auph implementation
#define AUPH_MP3
//#define AUPH_OGG

#include <auph/auph_impl.hpp>

#if defined(__ANDROID__)

#include <ek/app/Platform.h>

#endif

namespace ek::audio {

struct AudioSystem {
    int locks = 0;
    bool initialized = false;
};

static AudioSystem audioSystem{};

void initialize() {
    EK_DEBUG("audio initialize");
    EK_ASSERT(!audioSystem.initialized);
#if defined(__ANDROID__)
    auto activity = app::get_activity();
    auto assets = app::assetManagerRef();
    auph::setAndroidActivity(app::getJNIEnv, activity, assets);
#endif
    auph::init();
    audioSystem.initialized = true;
}

void shutdown() {
    EK_DEBUG("audio shutdown");
    EK_ASSERT(audioSystem.initialized);
    auph::shutdown();
    audioSystem.initialized = false;
}

void muteDeviceBegin() {
    EK_ASSERT(audioSystem.locks >= 0);
    if (audioSystem.locks == 0) {
        auph::setGain(auph::Bus_Master.id, 0.0f);
    }
    ++audioSystem.locks;
}

void muteDeviceEnd() {
    --audioSystem.locks;
    EK_ASSERT(audioSystem.locks >= 0);
    if (audioSystem.locks == 0) {
        auph::setGain(auph::Bus_Master.id, 1.0f);
    }
}

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
