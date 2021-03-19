#pragma once

// Enable access to assets via reading file for Windows and Apple (mac/ios)
// For Android and Emscripten internal mechanisms are used
#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)

// not optimized for reading app content with stdio

#elif defined(_WIN32) || defined(_WIN64) || defined(__APPLE__) || defined(__linux__)

#define EK_STATIC_RESOURCES_READ_FILE 1

#else // other platforms

#endif

#if EK_STATIC_RESOURCES_READ_FILE

#include <vector>
#include <cstdint>

namespace ek::internal {

std::vector<uint8_t> read_file_bytes(const char* path);

}
#endif
