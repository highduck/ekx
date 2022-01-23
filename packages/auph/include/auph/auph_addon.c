#ifndef AUPH_ADDON_IMPL
#define AUPH_ADDON_IMPL
#else
#error You should implement auph once
#endif

#include <auph/auph.h>
#include <ek/assert.h>

static int F2U(float x) {
    return (int) (x * AUPH_UNIT);
}

auph_voice auph_play_f(auph_buffer buffer,
                       float gain,
                       float pan,
                       float rate,
                       bool loop,
                       bool paused,
                       auph_bus bus) {
    int flags = 0;
    if (loop) flags |= AUPH_FLAG_LOOP;
    if (!paused) flags |= AUPH_FLAG_RUNNING;
    return auph_play(buffer, F2U(gain), F2U(pan + 1.0f), F2U(rate), flags, bus);
}

const char* auph_get_mixer_state_str(int state) {
    static const char* names[] = {"closed", "paused", "", "running"};
    return names[state & 0x3];
}

const char* auph_get_buffer_state_str(int state) {
    static const char* names[] = {"free", "loading", "", "loaded"};
    return names[state & 0x3];
    //+ ["", " streaming"][(state >>> 2) & 0x1];
}

void auph_set_gain(int bus_or_voice, float value) {
    auph_set(bus_or_voice, AUPH_PARAM_GAIN, F2U(value));
}

float auph_get_gain(int busOrVoice) {
    return (float) auph_get(busOrVoice, AUPH_PARAM_GAIN) / AUPH_UNIT;
}

void auph_set_pan(auph_voice voice, float pan) {
    auph_set(voice.id, AUPH_PARAM_PAN, F2U(pan + 1.0f));
}

void auph_set_rate(auph_voice voice, float rate) {
    auph_set(voice.id, AUPH_PARAM_RATE, F2U(rate));
}

void auph_set_pause(int name, bool value) {
    auph_set(name, (int) AUPH_PARAM_FLAGS | AUPH_FLAG_RUNNING, value ? 0 : 1);
}

void auph_set_loop(auph_voice voice, bool value) {
    auph_set(voice.id, (int) AUPH_PARAM_FLAGS | AUPH_FLAG_LOOP, value);
}

float auph_get_pan(auph_voice voice) {
    return (float) auph_get(voice.id, AUPH_PARAM_PAN) / (float) AUPH_UNIT - 1.0f;
}

float auph_get_rate(auph_voice voice) {
    return (float) auph_get(voice.id, AUPH_PARAM_RATE) / AUPH_UNIT;
}

bool auph_get_pause(auph_voice voice) {
    return !(auph_get(voice.id, AUPH_PARAM_STATE) & AUPH_FLAG_RUNNING);
}

bool auph_get_loop(auph_voice voice) {
    return !!(auph_get(voice.id, AUPH_PARAM_STATE) & AUPH_FLAG_LOOP);
}

float auph_get_current_time(auph_voice voice /* or mixer */) {
    return (float) auph_get(voice.id, AUPH_PARAM_CURRENT_TIME) / AUPH_UNIT;
}

bool auph_is_active(int name) {
    return !!(auph_get(name, AUPH_PARAM_STATE) & AUPH_FLAG_ACTIVE);
}

bool auph_is_buffer_loaded(auph_buffer buffer) {
    const int mask = AUPH_FLAG_ACTIVE | AUPH_FLAG_LOADED;
    return (auph_get(buffer.id, AUPH_PARAM_STATE) & mask) == mask;
}

/** Methods for Buffer **/
float auph_get_duration(int bufferOrVoice) {
    return (float) auph_get(bufferOrVoice, AUPH_PARAM_DURATION) / AUPH_UNIT;
}

void auph_pause(int name) {
    auph_set_pause(name, true);
}

void auph_resume(int name) {
    auph_set_pause(name, false);
}

int auph_global_mutes = 0;

void auph_mute_push() {
    EK_ASSERT(auph_global_mutes >= 0);
    if (auph_global_mutes == 0) {
        auph_set_gain(AUPH_BUS_MASTER.id, 0.0f);
    }
    ++auph_global_mutes;
}

void auph_mute_pop() {
    --auph_global_mutes;
    EK_ASSERT(auph_global_mutes >= 0);
    if (auph_global_mutes == 0) {
        auph_set_gain(AUPH_BUS_MASTER.id, 1.0f);
    }
}
