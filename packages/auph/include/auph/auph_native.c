#ifndef AUPH_NATIVE_IMPL
#define AUPH_NATIVE_IMPL
#else
#error You should implement auph once
#endif

#include "native/native.h"
#include "native/device.c"
#include "native/mixer.c"

#if defined(__ANDROID__) && defined(AUPH_SETUP_EK_APP)
#include <ek/app_native.h>
#endif

enum {
    AUPH_BUFFERS_MAX_COUNT = 128,
    AUPH_VOICES_MAX_COUNT = 64,
    AUPH_SCRATCH_BUFFER_SIZE = 2048
};

typedef struct auph_context {
    int state;
    auph_audio_device device;
    auph_mix_sample scratch[AUPH_SCRATCH_BUFFER_SIZE];
    auph_buffer_obj buffers[AUPH_BUFFERS_MAX_COUNT];
    auph_voice_obj voices[AUPH_VOICES_MAX_COUNT];
    auph_bus_obj bus[4];
} auph_context;

static void auph_mix(auph_context* ctx, auph_mix_sample* dest, uint32_t frames, uint32_t sampleRate) {
    const uint32_t samples = frames << 1;
    clear((float*) dest, samples);
    auph_render_voices(ctx->voices, ctx->bus, AUPH_VOICES_MAX_COUNT, dest, frames, sampleRate);
    clip((float*) dest, samples);
}

static void auph_mixer_playback(auph_audio_callback_data* data) {
    auph_context* ctx = (auph_context*) data->userData;
    const uint32_t sampleRate = (uint32_t) data->stream.sampleRate;
    // TODO: support F32 output
    auph_mix_sample* src = ctx->scratch;
    auph_mix(ctx, src, data->frames, sampleRate);

    uint32_t p = 0;
    int16_t* dest = (int16_t*) data->data;
    for (uint32_t i = 0; i < data->frames; ++i) {
        dest[p++] = (int16_t) (src->L * 0x7FFF);
        dest[p++] = (int16_t) (src->R * 0x7FFF);
        ++src;
    }
}

auph_context auph_ctx;

auph_audio_device* auph_get_audio_device(void) {
    return &auph_ctx.device;
}

auph_buffer_obj* auph_get_buffer_obj(int name) {
    const int index = name & AUPH_MASK_INDEX;
    if (index > 0 && index < AUPH_BUFFERS_MAX_COUNT) {
        auph_buffer_obj* obj = auph_ctx.buffers + index;
        if (obj->id == name) {
            return obj;
        }
    }
    return NULL;
}

auph_voice_obj* auph_get_voice_obj(int name) {
    const int index = name & AUPH_MASK_INDEX;
    if (index > 0 && index < AUPH_VOICES_MAX_COUNT) {
        auph_voice_obj* obj = auph_ctx.voices + index;
        if (obj->id == name) {
            return obj;
        }
    }
    return NULL;
}

auph_voice auph_get_next_voice(void) {
    const auph_voice_obj* objs = auph_ctx.voices;
    for (int i = 1; i < AUPH_VOICES_MAX_COUNT; ++i) {
        const auph_voice_obj* obj = objs + i;
        if (!obj->state) {
            return (auph_voice) {obj->id};
        }
    }
    return (auph_voice) {0};
}

auph_buffer auph_get_next_buffer(void) {
    const auph_buffer_obj* objs = auph_ctx.buffers;
    for (int i = 1; i < AUPH_BUFFERS_MAX_COUNT; ++i) {
        const auph_buffer_obj* obj = objs + i;
        // data slot is free
        if (!obj->state) {
            return (auph_buffer) {obj->id};
        }
    }
    return (auph_buffer) {0};
}

auph_bus_obj* auph_get_bus_obj(int name) {
    const int index = name & AUPH_MASK_INDEX;
    if (index >= 0 && index < 4) {
        auph_bus_obj* obj = auph_ctx.bus + index;
        if (obj->id == name) {
            return obj;
        }
    }
    return NULL;
}

void auph_setup(void) {
#if defined(__ANDROID__) && defined(AUPH_SETUP_EK_APP)
    auph_android_setup(ek_android_jni, ek_android_activity(), ek_android_assets_object());
#endif

    auph_context* ctx = &auph_ctx;
    ctx->state = AUPH_FLAG_ACTIVE;
    auph_audio_device_init(&ctx->device);
    ctx->device.onPlayback = auph_mixer_playback;
    ctx->device.userData = ctx;

    memset(ctx->scratch, 0, sizeof ctx->scratch);
    for (int i = 0; i < AUPH_BUFFERS_MAX_COUNT; ++i) {
        auph_buffer_obj_init(ctx->buffers + i, i | AUPH_TYPE_BUFFER);
    }
    for (int i = 0; i < AUPH_VOICES_MAX_COUNT; ++i) {
        auph_voice_obj_init(ctx->voices + i, i | AUPH_TYPE_VOICE);
    }
    for (int i = 0; i < 4; ++i) {
        auph_bus_obj_init(ctx->bus + i, i | AUPH_TYPE_BUS);
    }
}

void auph_shutdown(void) {
    auph_audio_device_term(&auph_ctx.device);
    auph_ctx.state = 0;
}

void auph_set(int name, int param, int value) {
    if (name == 0) {
        return;
    }

    auph_context* ctx = &auph_ctx;
    if (name == AUPH_MIXER && (param & AUPH_PARAM_FLAGS) && (param & AUPH_FLAG_RUNNING)) {
        if (!value && (ctx->state & AUPH_FLAG_RUNNING)) {
            ctx->state ^= AUPH_FLAG_RUNNING;
            auph_audio_device_stop(&ctx->device);
        } else if (value && !(ctx->state & AUPH_FLAG_RUNNING)) {
            ctx->state ^= AUPH_FLAG_RUNNING;
            auph_audio_device_start(&ctx->device);
        }
    }

    const int type = name & AUPH_MASK_TYPE;
    if (type == AUPH_TYPE_VOICE) {
        auph_voice_obj* obj = auph_get_voice_obj(name);
        if (obj) {
            if (param & AUPH_PARAM_FLAGS) {
                const bool enabled = value != 0;
                if (param & AUPH_FLAG_RUNNING) {
                    if ((obj->state & AUPH_FLAG_RUNNING) != enabled) {
                        obj->state ^= AUPH_FLAG_RUNNING;
                    }
                } else if (param & AUPH_FLAG_LOOP) {
                    if ((obj->state & AUPH_FLAG_LOOP) != enabled) {
                        obj->state ^= AUPH_FLAG_LOOP;
                    }
                }
            } else {
                switch (param) {
                    case AUPH_PARAM_GAIN:
                        obj->gain = value;
                        break;
                    case AUPH_PARAM_PAN:
                        obj->pan = value;
                        break;
                    case AUPH_PARAM_RATE:
                        obj->rate = value;
                        break;
                    case AUPH_PARAM_CURRENT_TIME:
                        // TODO: convert to seconds
                        obj->position = value;
                        break;
                    default:
                        break;
                }
            }
        }
    } else if (type == AUPH_TYPE_BUS) {
        auph_bus_obj* obj = auph_get_bus_obj(name);
        if (obj) {
            if (param & AUPH_PARAM_FLAGS) {
                if (param & AUPH_FLAG_RUNNING) {
                    const bool was = (obj->state & AUPH_FLAG_RUNNING) != 0;
                    if (was != (value != 0)) {
                        obj->state ^= AUPH_FLAG_RUNNING;
                    }
                }
            } else {
                switch (param) {
                    case AUPH_PARAM_GAIN:
                        if (obj->gain != value) {
                            obj->gain = value;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

int auph_get(int name, int param) {
    if (name == 0) {
        return 0;
    }
    auph_context* ctx = &auph_ctx;
    if (name == AUPH_MIXER) {
        if (param == AUPH_PARAM_STATE) {
            return ctx->state;
        } else if (param == AUPH_PARAM_SAMPLE_RATE) {
            // TODO:
            return 44100;
        }
        return 0;
    }
    const int type = name & AUPH_MASK_TYPE;
    if ((param & AUPH_PARAM_COUNT) && !(name & AUPH_MASK_INDEX)) {
        const int stateMask = param & AUPH_PARAM_STATE_MASK;
        int count = 0;
        if (type == AUPH_TYPE_VOICE) {
            for (int i = 1; i < AUPH_VOICES_MAX_COUNT; ++i) {
                if ((ctx->voices[i].state & stateMask) == stateMask) {
                    ++count;
                }
            }
        } else if (type == AUPH_TYPE_BUS) {
            for (int i = 0; i < 4; ++i) {
                if ((ctx->bus[i].state & stateMask) == stateMask) {
                    ++count;
                }
            }
        } else if (type == AUPH_TYPE_BUFFER) {
            for (int i = 1; i < AUPH_BUFFERS_MAX_COUNT; ++i) {
                if ((ctx->buffers[i].state & stateMask) == stateMask) {
                    ++count;
                }
            }
        }
        return count;
    }

    if (type == AUPH_TYPE_VOICE) {
        const auph_voice_obj* obj = auph_get_voice_obj(name);
        if (obj) {
            switch (param) {
                case AUPH_PARAM_STATE:
                    return obj->state;
                case AUPH_PARAM_GAIN:
                    return obj->gain;
                case AUPH_PARAM_PAN:
                    return obj->pan;
                case AUPH_PARAM_RATE:
                    return obj->rate;
                case AUPH_PARAM_DURATION:
                    // TODO:
                    return 0;
                case AUPH_PARAM_CURRENT_TIME:
                    // TODO: convert to seconds
                    //return (int)(obj->position * Unit);
                    return (int) obj->position;
                default:
                    //warn("not supported");
                    break;
            }
        }
        return 0;
    } else if (type == AUPH_TYPE_BUS) {
        const auph_bus_obj* obj = auph_get_bus_obj(name);
        if (obj) {
            switch (param) {
                case AUPH_PARAM_STATE:
                    return obj->state;
                case AUPH_PARAM_GAIN:
                    return obj->gain;
                default:
                    //warn("not supported");
                    break;
            }
        }
        return 0;
    } else if (type == AUPH_TYPE_BUFFER) {
        const auph_buffer_obj* obj = auph_get_buffer_obj(name);
        if (obj) {
            switch (param) {
                case AUPH_PARAM_STATE:
                    return obj->state;
                case AUPH_PARAM_DURATION:
                    return (int) obj->data.length;
                default:
                    //warn("param not supported");
                    break;
            }
        }
        return 0;
    }
    return 0;
}

auph_buffer auph_load(const char* filepath, int flags) {
    const auph_buffer buff = auph_get_next_buffer();
    if (buff.id) {
        auph_buffer_obj* buf = auph_get_buffer_obj(buff.id);
        if (buf && auph_buffer_obj_load(buf, filepath, flags)) {
            return buff;
        }
    }
    return (auph_buffer) {0};
}

auph_buffer auph_load_data(const void* data, int size, int flags) {
    if (data && size > 4) {
        const auph_buffer buff = auph_get_next_buffer();
        if (buff.id) {
            auph_buffer_obj* buf = auph_get_buffer_obj(buff.id);
            if (buf && auph_buffer_obj_load_data(buf, data, (uint32_t) size, flags)) {
                return buff;
            }
        }
    }
    return (auph_buffer) {0};
}

void auph_unload(auph_buffer buf) {
    auph_buffer_obj* obj = auph_get_buffer_obj(buf.id);
    if (obj) {
        auph_buffer_obj_unload(obj);
    }
}

auph_voice auph_play(auph_buffer buf, int gain, int pan, int rate, int flags, auph_bus bus) {
    auph_buffer_obj* bufferObj = auph_get_buffer_obj(buf.id);
    if (!bufferObj) {
        return (auph_voice) {0};
    }
    const auph_voice voice = auph_get_next_voice();
    if (voice.id == 0) {
        return voice;
    }
    auph_voice_obj* obj = &auph_ctx.voices[voice.id & AUPH_MASK_INDEX];
    obj->state = AUPH_FLAG_ACTIVE;
    if (flags & AUPH_FLAG_LOOP) {
        obj->state |= AUPH_FLAG_LOOP;
    }
    if (flags & AUPH_FLAG_RUNNING) {
        obj->state |= AUPH_FLAG_RUNNING;
    }
    obj->data = &bufferObj->data;
    obj->gain = gain;
    obj->pan = pan;
    obj->rate = rate;
    obj->position = 0.0;
    obj->bus = bus.id != 0 ? bus : AUPH_BUS_DEFAULT;
    return voice;
}

void auph_stop(int name) {
    const int type = name & AUPH_MASK_TYPE;
    if (type == AUPH_TYPE_VOICE) {
        auph_voice_obj* obj = auph_get_voice_obj(name);
        if (obj) {
            auph_voice_obj_stop(obj);
        }
    } else if (type == AUPH_TYPE_BUFFER) {
        const auph_buffer_obj* bufferObj = auph_get_buffer_obj(name);
        if (bufferObj) {
            const auph_buffer_data_source* pDataSource = &bufferObj->data;
            auph_voice_obj* voices = auph_ctx.voices;
            for (uint32_t i = 1; i < AUPH_VOICES_MAX_COUNT; ++i) {
                auph_voice_obj* voiceObj = voices + i;
                if (voiceObj->data == pDataSource) {
                    auph_voice_obj_stop(voiceObj);
                }
            }
        }
    }
}
