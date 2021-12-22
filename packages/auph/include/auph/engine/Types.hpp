#pragma once

typedef union auph_samples_data {
    // hackish const
    void* buffer;
    float* f32;
    int16_t* i16;
} auph_samples_data;

typedef struct auph_mix_sample {
    float L;
    float R;
} auph_mix_sample;

/**
 * stream reader function
 * reads num of frames and return number of read frames
 *
 * returns next dest pointer
 */
typedef auph_mix_sample* (* auph_source_reader_func)(auph_mix_sample*, const double, const double, const double,
                                                     const struct auph_buffer_data_source*,
                                                     auph_mix_sample volume);

typedef struct auph_buffer_data_source {
    void* stream_data;
    auph_samples_data data;
    // length in frames (samples / channels)
    uint32_t length;

    auph_sample_format format;
    uint32_t sample_rate;
    uint32_t channels;
    auph_source_reader_func reader;
} auph_buffer_data_source;

/** object's state **/
typedef struct auph_bus_obj {
    int id;
    int state;
    int gain;
} auph_bus_obj;

typedef struct auph_voice_obj {
    int id;
    int state;
    int gain;
    int pan;
    // playback speed
    int rate;
    auph_bus bus;

    // playback position in frames :(
    double position;

    const auph_buffer_data_source* data;
} auph_voice_obj;

typedef struct auph_buffer_obj {
    int id;
    int state;
    auph_buffer_data_source data;
    void* source_buffer_data;
} auph_buffer_obj;

/** common utilities **/

auph_source_reader_func auph_select_source_reader(auph_sample_format format, uint32_t channels, bool interpolate);

inline static void auph_bus_obj_init(auph_bus_obj* bus, int new_handle) {
    bus->id = new_handle;
    bus->state = AUPH_FLAG_RUNNING | AUPH_FLAG_ACTIVE;
    bus->gain = AUPH_UNIT;
}

inline static int auph_next_handle(int handle) {
    return ((handle + AUPH_ADD_VERSION) & AUPH_MASK_VERSION) | (handle & (AUPH_MASK_TYPE | AUPH_MASK_INDEX));
}

inline static float auph_bus_get_gain_f(const auph_bus_obj* bus) {
    return (bus->state & AUPH_FLAG_RUNNING) ? ((float) (bus->gain) / AUPH_UNIT) : 0.0f;
}

void auph_buffer_obj_unload(auph_buffer_obj* buf);

bool auph_buffer_obj_load(auph_buffer_obj* buf, const char* filepath, int flags);

bool auph_buffer_obj_load_data(auph_buffer_obj* buf, const void* data, uint32_t size, int flags);

inline static void auph_buffer_obj_init(auph_buffer_obj* buf, int new_handle) {
    memset(buf, 0, sizeof(auph_buffer_obj));
    buf->id = new_handle;
}

inline static void auph_voice_obj_init(auph_voice_obj* voice, int new_handle) {
    memset(voice, 0, sizeof(auph_voice_obj));
    voice->id = new_handle;
    voice->gain = AUPH_UNIT;
    voice->pan = AUPH_UNIT;
    voice->rate = AUPH_UNIT;
}

inline static void auph_voice_obj_stop(auph_voice_obj* voice) {
    auph_voice_obj_init(voice, auph_next_handle(voice->id));
}

inline static double auph_voice_get_rate_f64(const auph_voice_obj* voice) {
    return (double) voice->rate / AUPH_UNIT;
}

inline static float auph_voice_get_gain_f32(const auph_voice_obj* voice) {
    return (float) voice->gain / AUPH_UNIT;
}

inline static float auph_voice_get_pan_f32(const auph_voice_obj* voice) {
    return (float) voice->pan / AUPH_UNIT - 1.0f;
}
