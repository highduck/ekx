#ifndef AUPH_H
#define AUPH_H

#include "auph_api.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

auph_voice auph_play_f(auph_buffer buffer,
                       float gain,
                       float pan,
                       float rate,
                       bool loop,
                       bool paused,
                       auph_bus bus);

const char* auph_get_mixer_state_str(int state);
const char* auph_get_buffer_state_str(int state);

void auph_set_gain(int bus_or_voice, float value);

float auph_get_gain(int busOrVoice);

void auph_set_pan(auph_voice voice, float pan);

void auph_set_rate(auph_voice voice, float rate);

void auph_set_pause(int name, bool value);

void auph_set_loop(auph_voice voice, bool value);

float auph_get_pan(auph_voice voice);

float auph_get_rate(auph_voice voice);

bool auph_get_pause(auph_voice voice);

bool auph_get_loop(auph_voice voice);

float auph_get_current_time(auph_voice voice /* or mixer */);

bool auph_is_active(int name);

bool auph_is_buffer_loaded(auph_buffer buffer);

/** Methods for Buffer **/
float auph_get_duration(int bufferOrVoice);

void auph_pause(int name);

void auph_resume(int name);

#ifdef __cplusplus
}
#endif

#endif // AUPH_H