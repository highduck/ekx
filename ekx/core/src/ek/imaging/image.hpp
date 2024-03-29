#pragma once

#include <cstdint>
#include "../math/packed_color.hpp"
#include "../math/box.hpp"

namespace ek {

// Image Format:
// RGBA - GL like format
// ARGB is actually BGRA format, for example for Cairo output
class image_t {
public:

    image_t() = delete;

    image_t(const image_t& image);

    image_t(uint32_t width, uint32_t height);

    // delegate constructor
    image_t(uint32_t width, uint32_t height, uint8_t* data);

    ~image_t();

    void deallocate();

    [[nodiscard]]
    inline uint32_t width() const {
        return width_;
    }

    [[nodiscard]]
    inline uint32_t height() const {
        return height_;
    }

    [[nodiscard]]
    inline const uint8_t* data() const {
        return data_;
    }

    inline uint8_t* data() {
        return data_;
    }

    template<typename Pixel = abgr32_t>
    inline const Pixel* row(uint32_t y) const {
        return reinterpret_cast<const Pixel*>(data_ + y * stride());
    }

    template<typename Pixel = abgr32_t>
    inline Pixel* row(uint32_t y) {
        return reinterpret_cast<Pixel*>(data_ + y * stride());
    }

    template<typename T>
    rect_t<T> bounds() const {
        return {static_cast<T>(0),
                static_cast<T>(0),
                static_cast<T>(width_),
                static_cast<T>(height_)};
    }

    void assign(const image_t& src);

    [[nodiscard]]
    inline uint32_t stride() const {
        return width_ * 4u;
    }

    uint8_t* forget();

private:
    uint32_t width_;
    uint32_t height_;
    uint8_t* data_;
};

void fastBlurA8(uint8_t* data, int width, int height, int stride, float radius, int iterations, int strength);
image_t* decode_image_data(const void* data, size_t size, bool premultiplyAlpha = true);

}

