#include "texture.hpp"
#include "gl_debug.hpp"
#include <ek/imaging/image.hpp>
#include <cassert>

namespace ek {

texture_t::texture_t()
        : texture_t{false} {
}

texture_t::texture_t(bool is_cube_map)
        : type_{texture_type::color32},
          is_cube_map_{is_cube_map} {
    gl_texture_target_ = is_cube_map ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
    glGenTextures(1, &handle_);
    gl_check_error();
}

texture_t::~texture_t() {
    assert(handle_ != 0);
    glDeleteTextures(1, &handle_);
    gl_check_error();
}

static GLuint prev_texture_cube_map_binding_ = 0u;
static GLuint prev_texture_2d_binding_ = 0u;

void begin_texture_setup(GLuint texture_id, GLenum target_type) {
    glBindTexture(target_type, texture_id);
    gl_check_error();
#ifdef EK_WEBGL
    // TODO: const / restore state
    // glPixelStorei(UNPACK_PREMULTIPLY_ALPHA_WEBGL, 0);
#endif
}

void end_texture_setup(GLenum target_type) {
    glTexParameteri(target_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifndef EK_GLES2
    if (target_type == GL_TEXTURE_CUBE_MAP) {
        glTexParameteri(target_type, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }
#endif

    gl_check_error();

    if (target_type == GL_TEXTURE_CUBE_MAP && prev_texture_cube_map_binding_ != 0) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, prev_texture_cube_map_binding_);
        gl_check_error();
    } else if (target_type == GL_TEXTURE_2D && prev_texture_2d_binding_ != 0) {
        glBindTexture(GL_TEXTURE_2D, prev_texture_2d_binding_);
        gl_check_error();
    }
}

void texture_t::upload(const image_t& image) {
    upload_pixels(image.width(), image.height(), reinterpret_cast<const uint32_t*>(image.data()));
}

void texture_t::upload_pixels(uint32_t width, uint32_t height, const uint32_t* pixels_rgba32) {
    begin_texture_setup(handle_, gl_texture_target_);

    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsRgba32);
    glTexImage2D(gl_texture_target_, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_rgba32);
    gl_check_error();

    end_texture_setup(gl_texture_target_);
}

void texture_t::bind(int unit) const {
    glActiveTexture(GL_TEXTURE0 + (GLenum) unit);
    gl_check_error();

    glBindTexture(gl_texture_target_, handle_);
    gl_check_error();

    if (gl_texture_target_ == GL_TEXTURE_CUBE_MAP) {
        prev_texture_cube_map_binding_ = handle_;
    } else if (gl_texture_target_ == GL_TEXTURE_2D) {
        prev_texture_2d_binding_ = handle_;
    }
}

void texture_t::reset(uint32_t width, uint32_t height, texture_type type) {
    type_ = type;

    begin_texture_setup(handle_, gl_texture_target_);

    GLint internal_format = GL_RGBA;
    GLenum texture_format = GL_RGBA;
    GLenum pixel_type = GL_UNSIGNED_BYTE;

    switch (type_) {
        case texture_type::depth16:
            texture_format = GL_DEPTH_COMPONENT;
            internal_format = GL_DEPTH_COMPONENT16;
            pixel_type = GL_UNSIGNED_SHORT;
            break;
        case texture_type::depth24:
            texture_format = GL_DEPTH_COMPONENT;
#ifdef EK_GLES2
            internal_format = GL_DEPTH_COMPONENT16;
            internal_format = GL_DEPTH_COMPONENT;
            pixel_type = GL_UNSIGNED_SHORT;
#else
            internal_format = GL_DEPTH_COMPONENT24;
            pixel_type = GL_UNSIGNED_INT;
#endif
            break;
        default:
            break;
    }

    if (is_cube_map_) {
        for (int i = 0; i < 6; ++i) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internal_format, width, height, 0, texture_format,
                         pixel_type, nullptr);
        }
    } else {
        glTexImage2D(gl_texture_target_, 0, internal_format, width, height, 0, texture_format, pixel_type, nullptr);
    }
    gl_check_error();

    end_texture_setup(gl_texture_target_);
}

void texture_t::upload_cubemap(const std::array<image_t*, 6>& images) {
    begin_texture_setup(handle_, gl_texture_target_);
    for (int i = 0; i < 6; ++i) {
        auto& image = *(images[i]);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0, GL_RGBA,
                     image.width(), image.height(),
                     0, GL_RGBA,
                     GL_UNSIGNED_BYTE, image.data());
        gl_check_error();
    }
    end_texture_setup(gl_texture_target_);
}

}
