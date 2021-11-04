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

#include <vector>
#include <cstdint>
#include <ek/app/app.hpp>

namespace ek::internal {

inline std::vector<uint8_t> read_file_bytes(const char* path) {
    EKAPP_ASSERT(path != nullptr);
    std::vector<uint8_t> buffer;
    auto* stream = fopen(path, "rb");
    if (stream) {
        fseek(stream, 0, SEEK_END);
        buffer.resize(static_cast<size_t>(ftell(stream)));
        fseek(stream, 0, SEEK_SET);

        fread(buffer.data(), buffer.size(), 1u, stream);

        if (ferror(stream) != 0) {
            buffer.resize(0);
            buffer.shrink_to_fit();
        }

        fclose(stream);
    }
    else {
        EKAPP_LOG("Read file bytes error: file not found");
    }
    return buffer;
}

}

#endif
