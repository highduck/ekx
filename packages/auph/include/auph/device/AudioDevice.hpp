#pragma once

#include <stdint.h>

typedef enum auph_sample_format {
    AUPH_SAMPLE_FORMAT_F32 = 0,
    AUPH_SAMPLE_FORMAT_I16 = 1
} auph_sample_format;

typedef struct auph_audio_stream_info {
    uint32_t channels;
    uint32_t bytesPerSample;
    uint32_t bytesPerFrame;
    float sampleRate;
    auph_sample_format format;
} auph_audio_stream_info;

typedef struct auph_audio_callback_data {
    void* data;
    void* userData;
    uint32_t frames;
    auph_audio_stream_info stream;
} auph_audio_callback_data;

typedef enum auph_audio_device_status {
    AUPH_DEVICE_STATUS_SUCCESS = 0,
    AUPH_DEVICE_STATUS_INVALID_ARGUMENTS = 1,
    AUPH_DEVICE_STATUS_INITIALIZE_ERROR = 2
} auph_audio_device_status;

typedef void (* auph_audio_device_callback)(auph_audio_callback_data* data);

typedef struct auph_audio_device auph_audio_device;

auph_audio_device* auph_get_audio_device(void);

void auph_audio_device_init(auph_audio_device* device);

bool auph_audio_device_start(auph_audio_device* device);

bool auph_audio_device_stop(auph_audio_device* device);

// optional stop & destroy
void auph_audio_device_term(auph_audio_device* device);
