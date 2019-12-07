#pragma once

#include "gl_def.hpp"
#include <cstdint>
#include <ek/util/common_macro.hpp>

namespace ek {

enum class buffer_usage : GLenum {
    static_buffer = GL_STATIC_DRAW,
    dynamic_buffer = GL_STREAM_DRAW
};

enum class buffer_type : GLenum {
    vertex_buffer = GL_ARRAY_BUFFER,
    index_buffer = GL_ELEMENT_ARRAY_BUFFER
};

class buffer_object_t {
public:
    buffer_object_t(buffer_type type, buffer_usage usage);

    ~buffer_object_t();

    buffer_object_t() = delete;

    buffer_object_t(const buffer_object_t&) = delete;

    buffer_object_t(buffer_object_t&&) = delete;

    buffer_object_t& operator=(const buffer_object_t&) = delete;

    buffer_object_t& operator=(buffer_object_t&& t) = delete;

    void upload(const void* data, uint32_t size);
    void bind() const;

private:
    buffer_type type_;
    buffer_usage usage_;
    uint32_t size_;
    GLuint handle_;
};

}
