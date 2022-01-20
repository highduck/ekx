#include <ek/audio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct res_audio res_audio;

void audio_setup(void) {
    auph_setup();

    struct res_audio* R = &res_audio;
    rr_man_t* rr = &R->rr;

    rr->names = R->names;
    rr->data = R->data;
    rr->max = sizeof(R->data) / sizeof(R->data[0]);
    rr->num = 1;
    rr->data_size = sizeof(R->data[0]);
}

#ifdef __cplusplus
}
#endif
