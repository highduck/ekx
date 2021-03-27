#include "screen_recorder.hpp"

#include <stb_image_write.h>
#include <ek/math/box.hpp>
#include <ek/graphics/graphics.hpp>
#include <sys/stat.h>
#include <ek/app/app.hpp>

namespace ek {

void flip_image(const uint8_t* __restrict__ src, uint8_t* __restrict__ dst, uint32_t stride, uint32_t height) {
    for (uint32_t i = 0; i < height; ++i) {
        const uint32_t top = i * stride;
        const uint32_t bottom = (height - i - 1) * stride;
        for (uint32_t j = 0; j < stride; ++j) {
            dst[top + j] = src[bottom + j];
        }
    }
}

screen_recorder::screen_recorder(const std::string& filename, const rect_u& rect)
        : filename_{filename},
          buffer_{new uint8_t[rect.width * rect.height * 4]},
          buffer_swap_{new uint8_t[rect.width * rect.height * 4]},
          rect_{rect} {
    mkdir(("./" + filename).c_str(), (mode_t) 0733);
}

void screen_recorder::render() {
    int h = ek::app::g_app.drawable_size.y;
    glReadPixels(rect_.x, h - rect_.y - rect_.height, rect_.width, rect_.height, GL_RGBA, GL_UNSIGNED_BYTE,
                 buffer_);

    flip_image(buffer_, buffer_swap_, rect_.width * 4u, rect_.height);
    std::string file = filename_ + "/frame" + std::to_string(frame_) + ".png";
    // 3 - RGB
    // 3 * rect_.width - stride in bytes
    stbi_write_png(file.c_str(),
                   rect_.width,
                   rect_.height,
                   3,
                   buffer_swap_,
                   static_cast<int>(3 * rect_.width));
    ++frame_;
}

screen_recorder::~screen_recorder() {
    delete[] buffer_swap_;
    delete[] buffer_;
}

}
