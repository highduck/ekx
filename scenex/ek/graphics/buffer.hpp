#pragma once

#include "gl_def.hpp"
#include <cstdint>
#include <ek/util/common_macro.hpp>

namespace ek::graphics {

enum class buffer_usage : GLenum {
    static_buffer = GL_STATIC_DRAW,
    dynamic_buffer = GL_STREAM_DRAW
};

enum class buffer_type : GLenum {
    vertex_buffer = GL_ARRAY_BUFFER,
    index_buffer = GL_ELEMENT_ARRAY_BUFFER
};

class buffer_t : private disable_copy_assign_t {
public:
    buffer_t(buffer_type type, buffer_usage usage);

    ~buffer_t();

    void upload(const void* data, uint32_t size);

    void bind() const;

    [[nodiscard]]
    uint32_t getSize() const {
        return size_;
    }

    bool useDataOrphaning = false;

private:
    buffer_type type_;
    buffer_usage usage_;
    uint32_t size_;
    GLuint handle_;
};

}
