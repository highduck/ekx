#ifndef AUPH_NATIVE_BUFFER_IMPL
#define AUPH_NATIVE_BUFFER_IMPL
#else
#error You should implement auph once
#endif

#include "native.h"

#include <math.h>

#ifdef AUPH_WAV

#include "buffer_wav.c"

#endif

#ifdef AUPH_MP3

#include "buffer_mp3.c"

#endif

#ifdef AUPH_OGG

#include "buffer_ogg.c"

#endif

#if defined(__ANDROID__)

#include <android/asset_manager.h>

#endif

#if defined(__APPLE__)
#include <Foundation/Foundation.h>
#endif

/**
 * Small JS snippet to get FourCC code from 4-char string
 * ```javascript
 * fourCC = s => "0x" + [...s].reduce((v,c,i) => v | c.charCodeAt() << ((3 - i) << 3), 0).toString(16);
 * fourCC("OggS") === "0x4f676753"
 * ```
 */
enum auph_four_cc {
    AUPH_FOUR_CC_OGGS = 0x4f676753,
    AUPH_FOUR_CC_RIFF = 0x52494646,
    AUPH_FOUR_CC_WAVE = 0x57415645,
    AUPH_FOUR_CC_ID3 = 0x49443300
};

void auph_buffer_obj_unload(auph_buffer_obj* buf) {
    free(buf->data.data.buffer);
    free(buf->data.stream_data);
    free(buf->source_buffer_data);

    int id = auph_next_handle(buf->id);
    memset(buf, 0, sizeof(auph_buffer_obj));
    buf->id = id;
}

#if defined(__APPLE__)

static const char* auph_get_file_path_from_bundle(const char* filepath) {
    NSString * pathToAsset = [NSString stringWithUTF8String: filepath];
    NSString * pathFromBundle = [[NSBundle mainBundle] pathForResource:pathToAsset ofType:nil];
    const char* filepathFromBundle = [pathFromBundle cStringUsingEncoding:NSASCIIStringEncoding];
    return filepathFromBundle ? filepathFromBundle : filepath;
}

#endif

static const char* auph_get_extension(const char* filepath) {
    const char* lastDot = filepath;
    while (*filepath != '\0') {
        if (*filepath == '.') {
            lastDot = filepath;
        }
        filepath++;
    }
    return lastDot;
}

static bool auph_load_to_buffer(auph_buffer_data_source* dataSource, const char* filepath, int flags) {

    const char* e = auph_get_extension(filepath);
#ifdef AUPH_MP3
    if (e[1] == 'm' && e[2] == 'p' && e[3] == '3') {
        if (flags & AUPH_FLAG_STREAM) {
            return auph_open_file_stream_mp3(filepath, dataSource);
        } else {
            return auph_load_file_mp3(filepath, dataSource);
        }
    }
#endif // AUPH_MP3

#ifdef AUPH_OGG
    if (e[1] == 'o' && e[2] == 'g' && e[3] == 'g') {
        return auph_load_file_ogg(filepath, dataSource, flags & AUPH_FLAG_STREAM);
    }
#endif // AUPH_OGG

#ifdef AUPH_WAV
    if (e[1] == 'w' && e[2] == 'a' && e[3] == 'v') {
        if (flags & AUPH_FLAG_STREAM) {
            return auph_open_file_stream_wav(filepath, dataSource);
        } else {
            return auph_load_file_wav(filepath, dataSource);
        }
    }
#endif // AUPH_WAV
    return false;
}

bool auph_load_memory_to_buffer(auph_buffer_data_source* dataSource, const void* data, uint32_t size, int flags) {
    if (size < 4) {
        return false;
    }
    const uint8_t* u8 = (const uint8_t*) data;
    const uint32_t fourCC = (u8[0] << 24) | (u8[1] << 16) | (u8[2] << 8) | u8[3];

#ifdef AUPH_OGG
    if (fourCC == AUPH_FOUR_CC_OGGS) {
        return auph_load_memory_ogg(data, size, dataSource, !!(flags & AUPH_FLAG_STREAM));
    }
#endif

#ifdef AUPH_WAV
    if (fourCC == AUPH_FOUR_CC_RIFF || fourCC == AUPH_FOUR_CC_WAVE) {
        if (flags & AUPH_FLAG_STREAM) {
            return auph_open_memory_stream_wav(data, size, dataSource);
        } else {
            return auph_load_memory_wav(data, size, dataSource);
        }
    }
#endif

#ifdef AUPH_MP3
    if ((fourCC & 0xFFFFFF00) == AUPH_FOUR_CC_ID3 || (fourCC & 0xFFE00000) == 0xFFE00000 /* 11-bit sync */) {
        if (flags & AUPH_FLAG_STREAM) {
            return auph_open_memory_stream_mp3(data, size, dataSource);
        } else {
            return auph_load_memory_mp3(data, size, dataSource);
        }
    }
#endif

    return false;
}

bool auph_buffer_obj_load(auph_buffer_obj* buf, const char* filepath, int flags) {
#if defined(__ANDROID__)
    if (auph_android.assets) {
        AAsset* asset = AAssetManager_open(auph_android.assets, filepath, AASSET_MODE_BUFFER);
        if (asset) {
            const uint8_t* dataBuffer = (const uint8_t*) AAsset_getBuffer(asset);
            off_t size = AAsset_getLength(asset);
            bool result = auph_buffer_obj_load_data(buf, dataBuffer, size, flags | AUPH_FLAG_COPY);
            AAsset_close(asset);
            return result;
        }
    }
#endif

#ifdef __APPLE__
    filepath = auph_get_file_path_from_bundle(filepath);
#endif

    const bool result = auph_load_to_buffer(&buf->data, filepath, flags);
    if (result) {
        buf->state = AUPH_FLAG_ACTIVE | AUPH_FLAG_LOADED;
        if (flags & AUPH_FLAG_STREAM) {
            buf->state |= AUPH_FLAG_STREAM;
        }
    }
    return result;
}

bool auph_buffer_obj_load_data(auph_buffer_obj* buf, const void* pData, uint32_t size, int flags) {
    // check if we need to preserve loaded data, for example we need to read encoded data continuously while playing
    const int flagsToCopy = AUPH_FLAG_STREAM | AUPH_FLAG_COPY;
    if ((flags & flagsToCopy) == flagsToCopy) {
        buf->source_buffer_data = malloc(size);
        memcpy(buf->source_buffer_data, pData, size);
        pData = buf->source_buffer_data;
    }

    const bool result = auph_load_memory_to_buffer(&buf->data, pData, size, flags);
    if (result) {
        buf->state = AUPH_FLAG_ACTIVE | AUPH_FLAG_LOADED;
        if (flags & AUPH_FLAG_STREAM) {
            buf->state |= AUPH_FLAG_STREAM;
        }
    } else {
        free(buf->source_buffer_data);
        buf->source_buffer_data = NULL;
    }
    return result;
}
