#include <auph/auph.h>

#include <ek/log.h>
#include <ek/assert.h>

#if 1
// for refactoring enable other formats
#define AUPH_WAV
#define AUPH_OGG
#endif

#define AUPH_MP3

#include <auph/auph_impl.hpp>

#if defined(__ANDROID__)

#include <ek/app_native.h>

#endif

int ek_audio_mutes = 0;

void ek_audio_setup() {
    EK_DEBUG("audio initialize");
#if defined(__ANDROID__)
    auto activity = ek_android_activity();
    auto assets = ek_android_assets_object();
    auph_android_setup(ek_android_jni, activity, assets);
#endif
    auph_init();
}

void ek_audio_shutdown() {
    EK_DEBUG("audio shutdown");
    auph_shutdown();
}

void ek_audio_mute_push() {
    EK_ASSERT(ek_audio_mutes >= 0);
    if (ek_audio_mutes == 0) {
        auph_set_gain(AUPH_BUS_MASTER.id, 0.0f);
    }
    ++ek_audio_mutes;
}

void ek_audio_mute_pop() {
    --ek_audio_mutes;
    EK_ASSERT(ek_audio_mutes >= 0);
    if (ek_audio_mutes == 0) {
        auph_set_gain(AUPH_BUS_MASTER.id, 1.0f);
    }
}