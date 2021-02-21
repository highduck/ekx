#include "decoder.hpp"

#include "image.hpp"
#include "drawing.hpp"
#include <ek/util/logger.hpp>
#include "../assert.hpp"
#include "../Allocator.hpp"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#define STBI_NO_STDIO
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG

static ek::StdAllocator stbi_heap{"stb_image"};

#define STBI_MALLOC(size)               stbi_heap.alloc(size, 4)
#define STBI_REALLOC(ptr, newSize)      stbi_heap.realloc(ptr, newSize, 4)
#define STBI_FREE(ptr)                  stbi_heap.dealloc(ptr)

#include <stb_image.h>

#endif

namespace ek {

image_t* decode_image_data(const void* data, size_t size) {
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
        premultiply_image(*result);
    } else {
        EK_ERROR("image decoding error: %s", stbi_failure_reason());
    }
    return result;
}

}