#include "image.hpp"

#include "../Allocator.hpp"
#include "ImageSubSystem.hpp"

namespace ek {

image_t::image_t(const image_t& image) : width_{image.width_},
                                         height_{image.height_} {
    data_ = (uint8_t*) imaging::allocator.alloc(image.width_ * image.height_ * 4u, sizeof(void*));
    assign(image);
}

image_t::image_t(uint32_t width, uint32_t height) : width_(width),
                                                    height_(height) {
    auto size = width * height * 4u;
    data_ = (uint8_t*) imaging::allocator.alloc(size, sizeof(void*));
    memory::clear(data_, size);
}

image_t::image_t(uint32_t width, uint32_t height, uint8_t* data) : width_{width},
                                                                   height_{height},
                                                                   data_{data} {
}

image_t::~image_t() {
    deallocate();
}

void image_t::assign(const image_t& src) {
    memory::copy(data_, src.data_, src.stride() * src.height_);
}

void image_t::deallocate() {
    if (data_) {
        imaging::allocator.dealloc(data_);
    }
    data_ = nullptr;
}

uint8_t* image_t::forget() {
    auto* ptr = data_;
    data_ = nullptr;
    width_ = 0u;
    height_ = 0u;
    return ptr;
}

}