#pragma once

#include "image.hpp"
#include "drawing.hpp"
#include <ek/debug.hpp>
#include "../assert.hpp"
#include "../Allocator.hpp"
#include "ImageSubSystem.hpp"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

// check arm64-v8a or armv7
#if defined(__APPLE__) && (defined(__aarch64__) || defined(__arm__))
#define STBI_NEON
#endif

#if defined(__ANDROID__) && defined(__aarch64__)
#define STBI_NEON
#endif

#define STBI_NO_STDIO
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG

#define STBI_ASSERT(e)   EK_ASSERT(e)
#define STBI_MALLOC(size)                           ::ek::imaging::allocator.alloc(size, sizeof(void*))
#define STBI_REALLOC_SIZED(ptr, oldSize, newSize)   ::ek::imaging::allocator.reallocate(ptr, oldSize, newSize, sizeof(void*))
#define STBI_FREE(ptr)                              ::ek::imaging::allocator.dealloc(ptr)

#include <stb_image.h>

#endif

namespace ek {

    image_t *decode_image_data(const void *data, size_t size, bool premultiplyAlpha) {

        EK_TRACE("decode image: begin");
        EK_ASSERT(size > 0);

        image_t *result = nullptr;

        int w = 0;
        int h = 0;
        int channels = 0;
        auto *image_data = stbi_load_from_memory(static_cast<const uint8_t *>(data),
                                                 static_cast<int>(size),
                                                 &w, &h, &channels, 4);


        if (image_data != nullptr) {
            result = new image_t(static_cast<uint32_t>(w),
                                 static_cast<uint32_t>(h),
                                 image_data);
            if (premultiplyAlpha) {
                EK_TRACE("decode image: premultiply alpha");
                premultiply_image(*result);
            }
        } else {
            EK_ERROR("image decoding error: %s", stbi_failure_reason());
        }
        EK_TRACE("decode image: end");
        return result;
    }

}