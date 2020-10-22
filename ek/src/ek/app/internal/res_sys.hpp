#pragma once

// Enable access to assets via reading file for Windows and Apple (mac/ios)
// For Android and Emscripten internal mechanisms are used
#if defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)

#define EK_STATIC_RESOURCES_READ_FILE 1

#endif //#else defined(__EMSCRIPTEN__) || defined(__ANDROID__)

#if EK_STATIC_RESOURCES_READ_FILE

#include <vector>
#include <cstdint>

namespace ek::internal {

std::vector<uint8_t> read_file_bytes(const char* path);

}
#endif
