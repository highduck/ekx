#include "image.hpp"

#include <ek/logger.hpp>
#include <algorithm>

namespace ek {

image_t::image_t(const image_t& image)
        : width_{image.width_},
          height_{image.height_},
          data_{new uint8_t[image.width_ * image.height_ * 4u]} {
    assign(image);
}

image_t::image_t(uint32_t width, uint32_t height)
        : width_(width),
          height_(height),
          data_{new uint8_t[width * height * 4u]} {
    std::fill_n(data_, width * height * 4u, 0u);
}

image_t::image_t(uint32_t width, uint32_t height, uint8_t* data, bool use_c_free)
        : width_{width},
          height_{height},
          data_{data},
          use_c_free_{use_c_free} {
}

void image_t::assign(const image_t& src) {
    std::copy_n(src.data_, src.stride() * src.height_, data_);
}

void image_t::deallocate() {
    if (data_) {
        if (use_c_free_) {
            free(data_);
        } else {
            delete[] data_;
        }
    }
    data_ = nullptr;
}

uint8_t* image_t::forget() {
    auto* ptr = data_;
    data_ = nullptr;
    use_c_free_ = false;
    width_ = 0u;
    height_ = 0u;
    return ptr;
}

}