#pragma once

#include "auph/auph.h"
#include <ek/util/Type.hpp>

#ifdef __cplusplus
extern "C" {
#endif

void ek_audio_setup(void);

void ek_audio_shutdown(void);

void ek_audio_mute_push(void);

void ek_audio_mute_pop(void);

#ifdef __cplusplus
}
#endif

namespace ek {

class AudioResource {
public:
    AudioResource() = default;

    ~AudioResource();

    void load(const char* filepath, bool streaming);

    void unload();

    auph_buffer buffer{};
};

EK_DECLARE_TYPE(AudioResource);

}
