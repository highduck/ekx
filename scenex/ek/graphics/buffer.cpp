#include <ek/graphics/buffer.hpp>

#include <cassert>
#include "gl_debug.hpp"

namespace ek::graphics {

buffer_t::buffer_t(buffer_type type, buffer_usage usage)
        : type_{type},
          usage_{usage},
          size_{0u},
          handle_{0u} {
    glGenBuffers(1, &handle_);
    gl::check_error();
}

buffer_t::~buffer_t() {
    assert(handle_ != 0);
    glDeleteBuffers(1, &handle_);
    gl::check_error();
}

void buffer_t::bind() const {
    const auto type = static_cast<GLenum>(type_);
    glBindBuffer(type, handle_);
    gl::check_error();
}

void buffer_t::upload(const void* data, uint32_t size) {
    bind();

    const auto usage = static_cast<GLenum>(usage_);
    const auto type = static_cast<GLenum>(type_);

    if (size > size_) {
        glBufferData(type, size, data, usage);
        gl::check_error();

        size_ = size;
    } else {
        glBufferData(type, size_, nullptr, usage);
        gl::check_error();

        glBufferSubData(type, 0, size, data);
        gl::check_error();
    }
}

}