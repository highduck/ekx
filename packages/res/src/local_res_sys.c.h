// Enable access to assets via reading file for Windows and Apple (mac/ios)
// For Android and Emscripten internal mechanisms are used
#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)

// not optimized for reading app content with stdio

#elif defined(_WIN32) || defined(_WIN64) || defined(__APPLE__) || defined(__linux__)

#define EK_STATIC_RESOURCES_READ_FILE

#else // other platforms

#endif

#ifdef EK_STATIC_RESOURCES_READ_FILE

#include <stdint.h>
#include <stdio.h>
#include <ek/app.h>
#include "map_file.c.h"

static int ek_local_res__read_file(const char* path, uint8_t** outBuffer, size_t* outLength) {
    if (path == NULL) {
        return 3;
    }

    int32_t result = 0;

    *outBuffer = NULL;
    *outLength = 0;

    FILE* stream = fopen(path, "rb");
    if (!stream) {
        result = 1;
    } else {
        fseek(stream, 0, SEEK_END);
        *outLength = (size_t)ftell(stream);
        fseek(stream, 0, SEEK_SET);

        *outBuffer = (uint8_t*) malloc(*outLength);
        fread(*outBuffer, *outLength, 1u, stream);

        if (ferror(stream) != 0) {
            free(*outBuffer);
            *outBuffer = NULL;
            *outLength = 0;
            result = 2;
        }

        fclose(stream);
    }

    return result;
}

#ifdef EK_HAS_MAP_FILE

static void closeMappedFile(ek_local_res* lr) {
    if (lr->buffer) {
        munmap(lr->buffer, lr->length);
        lr->buffer = NULL;
        lr->length = 0;
    }
}

#endif

void closeMemoryFile(ek_local_res* lr) {
    free(lr->buffer);
    lr->buffer = NULL;
    lr->length = 0;
}

int ek_local_res_get_file_platform(const char* path, ek_local_res* lr) {
#ifdef __APPLE__
    char buf[1024];
    path = ek_app_ns_bundle_path(path, buf, sizeof(buf));
#endif // __APPLE__

    int result = 0;
#ifdef EK_HAS_MAP_FILE
    result = ek_local_res__map_file(path, &lr->buffer, &lr->length);
    if (result == 0) {
        lr->closeFunc = closeMappedFile;
        lr->status = 0;
        return 0;
    }
#endif

    result = ek_local_res__read_file(path, &lr->buffer, &lr->length);
    lr->status = result;
    lr->closeFunc = closeMemoryFile;
    return result;
}

#endif // EK_STATIC_RESOURCES_READ_FILE
