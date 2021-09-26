#pragma once

#include "image.hpp"
#include "drawing.hpp"
#include <ek/debug.hpp>
#include "../assert.hpp"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

// enable NEON for all ARM targets on Android and iOS
#if defined(__aarch64__) || defined(__arm__)
#if defined(__APPLE__) || defined(__ANDROID__)
#define STBI_NEON
#endif
#endif // arm

#ifndef EK_TOOLS
#define STBI_NO_STDIO
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#endif

#ifndef NDEBUG
#define STBI_NO_FAILURE_STRINGS
#else
#define STBI_FAILURE_USERMSG
#endif // DEBUG

#define STBI_ASSERT(e)   EK_ASSERT(e)

#include <stb/stb_image.h>

#endif

namespace ek {

image_t* decode_image_data(const void* data, size_t size, bool pma) {

    EK_DEBUG("decode image: begin");
    EK_ASSERT(size > 0);

    image_t* result = nullptr;

    int w = 0;
    int h = 0;
    int channels = 0;
    auto* image_data = stbi_load_from_memory(static_cast<const uint8_t*>(data),
                                             static_cast<int>(size),
                                             &w, &h, &channels, 4);


    if (image_data != nullptr) {
        result = new image_t(static_cast<uint32_t>(w),
                             static_cast<uint32_t>(h),
                             image_data);
        if (pma) {
            EK_DEBUG("decode image: premultiply alpha");
            EK_PROFILE_SCOPE("pma");
            premultiplyAlpha(*result);
        }
    } else {
#ifndef NDEBUG
        EK_ERROR_F("image decoding error: %s", stbi_failure_reason());
#endif
    }
    EK_DEBUG("decode image: end");
    return result;
}

}