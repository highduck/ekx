#include "decoder.hpp"

#include "image.hpp"
#include "drawing.hpp"
#include <ek/fs/path.hpp>
#include <ek/logger.hpp>
#include <stb/stb_image.h>

namespace ek {

image_t* decode_image_data(const array_buffer& buffer) {
    assert(buffer.data());
    assert(!buffer.empty());

    image_t* result = nullptr;

    int w = 0;
    int h = 0;
    int channels = 0;
    auto* image_data = stbi_load_from_memory(buffer.data(),
                                             static_cast<int>(buffer.size()),
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