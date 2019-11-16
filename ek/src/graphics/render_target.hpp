#pragma once

#include <ek/utility/common_macro.hpp>
#include "gl_def.hpp"
#include "texture.hpp"

namespace ek {

class render_target_t : private disable_copy_assign_t {
public:

    render_target_t(uint32_t width, uint32_t height, texture_type type = texture_type::color32);

    ~render_target_t();

    void set();

    void reset(uint32_t width, uint32_t height);

    void unset();

    void clear();

    [[nodiscard]] uint32_t width() const {
        return width_;
    }

    [[nodiscard]] uint32_t height() const {
        return height_;
    }

    [[nodiscard]] const texture_t* texture() const {
        return texture_;
    }

private:
    uint32_t width_;
    uint32_t height_;
    texture_t* texture_;
    GLuint frame_buffer_handle_ = 0u;
    GLuint render_buffer_handle_ = 0u;
};

}