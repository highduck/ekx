#pragma once

// Enable access to assets via reading file for Windows and Apple (mac/ios)
// For Android and Emscripten internal mechanisms are used
#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)

// not optimized for reading app content with stdio
#define EK_STATIC_RESOURCES_READ_FILE  (0)

#elif defined(_WIN32) || defined(_WIN64) || defined(__APPLE__) || defined(__linux__)

#define EK_STATIC_RESOURCES_READ_FILE  (1)

#else // other platforms

#endif

#if EK_STATIC_RESOURCES_READ_FILE

#include <cstdint>
#include <cstdio>
#include "MapFile.hpp"

namespace ek {

int readFile(const char* path, uint8_t** outBuffer, size_t* outLength) {
    if (path == nullptr) {
        return 3;
    }

    int32_t result = 0;

    *outBuffer = nullptr;
    *outLength = 0;

    auto* stream = fopen(path, "rb");
    if (!stream) {
        result = 1;
    } else {
        fseek(stream, 0, SEEK_END);
        *outLength = static_cast<size_t>(ftell(stream));
        fseek(stream, 0, SEEK_SET);

        *outBuffer = (uint8_t*) malloc(*outLength);
        fread(*outBuffer, *outLength, 1u, stream);

        if (ferror(stream) != 0) {
            free(*outBuffer);
            *outBuffer = nullptr;
            *outLength = 0;
            result = 2;
        }

        fclose(stream);
    }

    return result;
}

#if EK_HAS_MAP_FILE

void closeMappedFile(LocalResource* lr) {
    if (lr->buffer) {
        munmap(lr->buffer, lr->length);
        lr->buffer = nullptr;
        lr->length = 0;
    }
}

#endif

void closeMemoryFile(LocalResource* lr) {
    free(lr->buffer);
    lr->buffer = nullptr;
    lr->length = 0;
}

int getFile(const char* path, LocalResource* lr) {
#if EK_HAS_MAP_FILE
    auto mapFileResult = mapFile(path, &lr->buffer, &lr->length);
    if (mapFileResult == 0) {
        lr->closeFunc = closeMappedFile;
        lr->status = 0;
        return 0;
    }
#endif
    auto openFileResult = readFile(path, &lr->buffer, &lr->length);
    lr->status = openFileResult;
    lr->closeFunc = closeMemoryFile;
    return openFileResult;
}

}

#endif
