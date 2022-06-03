#ifndef EK_AUDIO_H
#define EK_AUDIO_H

#include <auph/auph.h>
#include <ek/rr.h>

#ifdef __cplusplus
extern "C" {
#endif

void audio_setup(void);

struct res_audio {
    string_hash_t names[64];
    auph_buffer data[64];
    rr_man_t rr;
};

extern struct res_audio res_audio;

#define R_AUDIO(name) REF_NAME(res_audio, name)

#ifdef __cplusplus
}
#endif

#endif // EK_AUDIO_H
