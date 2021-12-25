#ifndef AUPH_H
#define AUPH_H

#include <stdbool.h>

enum {
    AUPH_MASK_INDEX = 0x000000FF,
    AUPH_ADD_VERSION = 0x00000100,
    AUPH_MASK_VERSION = 0x00FFFF00,
    AUPH_MASK_TYPE = 0x30000000,
    AUPH_UNIT = 1024,
};

typedef struct auph_buffer {
    int id;
} auph_buffer;

typedef struct auph_voice {
    int id;
} auph_voice;

typedef struct auph_bus {
    int id;
} auph_bus;

/** Object Type **/
enum auph_type {
    AUPH_TYPE_MIXER = 0,
    AUPH_TYPE_BUS = 1 << 28,
    AUPH_TYPE_BUFFER = 2 << 28,
    AUPH_TYPE_VOICE = 3 << 28,
    AUPH_TYPE_FORCE_I32_ = 0x7FFFFFFF
};

#define AUPH_BUS_MASTER   ((auph_bus){AUPH_TYPE_BUS | 0})
#define AUPH_BUS_SOUND   ((auph_bus){AUPH_TYPE_BUS | 1})
#define AUPH_BUS_MUSIC   ((auph_bus){AUPH_TYPE_BUS | 2})
#define AUPH_BUS_SPEECH   ((auph_bus){AUPH_TYPE_BUS | 3})

#define AUPH_BUS_DEFAULT   AUPH_BUS_SOUND

enum {
    AUPH_MIXER = AUPH_TYPE_MIXER | 1
};

enum auph_param {
    AUPH_PARAM_STATE = 0,
    AUPH_PARAM_GAIN = 1,
    AUPH_PARAM_PAN = 2,
    AUPH_PARAM_RATE = 3,
    AUPH_PARAM_CURRENT_TIME = 4,
    AUPH_PARAM_SAMPLE_RATE = 5,
    AUPH_PARAM_DURATION = 6,

    AUPH_PARAM_STATE_MASK = (1 << 7) - 1,
    AUPH_PARAM_FLAGS = 1 << 7,
    // counts object by state mask
    AUPH_PARAM_COUNT = 1 << 8,

    AUPH_PARAM_FORCE_I32_ = 0x7FFFFFFF
};

enum auph_flag {
    AUPH_FLAG_ACTIVE = 1,
    // Voice: playback is running (un-paused)
    // Buffer: buffer is loaded and ready for reading from
    // Bus: connected state
    // Mixer: is not paused
    AUPH_FLAG_RUNNING = 2,
    AUPH_FLAG_LOOP = 4,

    // Buffer Flags
    AUPH_FLAG_LOADED = 2,
    AUPH_FLAG_STREAM = 4,

    // Copy Flag for buffer
    AUPH_FLAG_COPY = 8,

    AUPH_FLAG_FORCE_I32_ = 0x7FFFFFFF
};

#ifdef __cplusplus
extern "C" {
#endif

void auph_setup(void);

void auph_shutdown(void);

auph_buffer auph_load(const char* filepath, int flags);

auph_buffer auph_load_data(const void* data, int size, int flags);

void auph_unload(auph_buffer buffer);

auph_voice auph_play(auph_buffer buffer, int gain, int pan, int rate, int flags, auph_bus bus);

void auph_stop(int name);

void auph_set(int name, int param, int value);

int auph_get(int name, int param);

int auph_vibrate(int duration_millis);

// addon functions

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

void auph_mute_push();

void auph_mute_pop();

#ifdef __cplusplus
}
#endif

#endif // AUPH_H