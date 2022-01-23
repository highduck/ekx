#ifndef AUPH_NATIVE_DEVICE_NULL_IMPL
#define AUPH_NATIVE_DEVICE_NULL_IMPL
#else
#error You should implement auph once
#endif

#include "native.h"

struct auph_audio_device {
    auph_audio_stream_info playbackStreamInfo;
    auph_audio_device_callback onPlayback;
    void* userData;
};

int auph_vibrate(int millis) {
    (void) millis;
    return 1;
}

void auph_audio_device_init(auph_audio_device* device) {
    (void) device;
}

bool auph_audio_device_start(auph_audio_device* device) {
    (void) device;
    return true;
}

bool auph_audio_device_stop(auph_audio_device* device) {
    (void) device;
    return true;
}

// optional stop & destroy
void auph_audio_device_term(auph_audio_device* device) {
    (void) device;
}

