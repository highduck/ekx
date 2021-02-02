#include "decoder.hpp"

#include "image.hpp"
#include "drawing.hpp"
#include <ek/util/logger.hpp>
#include <cassert>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#define STBI_NO_STDIO
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG

#include <stb_image.h>

#endif

namespace ek {

image_t* decode_image_data([[maybe_unused]] const std::vector<uint8_t>& data) {
    assert(!data.empty());

    image_t* result = nullptr;

    int w = 0;
    int h = 0;
    int channels = 0;
    auto* image_data = stbi_load_from_memory(data.data(),
                                             static_cast<int>(data.size()),
                                             &w, &h, &channels, 4);
    if (image_data != nullptr) {
        result = new image_t(
                static_cast<uint32_t>(w),
                static_cast<uint32_t>(h),
                image_data,
                true);
        premultiply_image(*result);
    } else {
        EK_ERROR << "image decoding error: " << stbi_failure_reason();
    }
    return result;
}

}