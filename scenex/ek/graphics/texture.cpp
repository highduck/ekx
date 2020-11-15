#include "texture.hpp"
#include "gl_debug.hpp"
#include "graphics.hpp"
#include "gl_def.hpp"
#include <ek/imaging/image.hpp>
#include <cassert>

#ifndef GL_INTENSITY
#define GL_INTENSITY                0x8049
#endif

#ifndef GL_LUMINANCE
#define GL_LUMINANCE                      0x1909
#endif

namespace ek::graphics {

texture_t::texture_t()
        : texture_t{false} {
}

texture_t::texture_t(bool is_cube_map)
        : type_{texture_type::color32},
          is_cube_map_{is_cube_map} {
    gl_texture_target_ = is_cube_map ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
    glGenTextures(1, &handle_);
    gl::check_error();
}

texture_t::~texture_t() {
    assert(handle_ != 0);
    glDeleteTextures(1, &handle_);
    gl::check_error();
}

static GLuint prev_texture_cube_map_binding_ = 0u;
static GLuint prev_texture_2d_binding_ = 0u;

void begin_texture_setup(GLuint texture_id, GLenum target_type) {
    glBindTexture(target_type, texture_id);
    gl::check_error();
#ifdef EK_WEBGL
    // TODO: const / restore state
    // glPixelStorei(UNPACK_PREMULTIPLY_ALPHA_WEBGL, 0);
#endif
}

void end_texture_setup(GLenum target_type, GLenum minFilter = GL_LINEAR) {
    glTexParameteri(target_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target_type, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(target_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifndef EK_GLES2
    if (target_type == GL_TEXTURE_CUBE_MAP) {
        glTexParameteri(target_type, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }
#endif

    gl::check_error();

    if (target_type == GL_TEXTURE_CUBE_MAP && prev_texture_cube_map_binding_ != 0) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, prev_texture_cube_map_binding_);
        gl::check_error();
    } else if (target_type == GL_TEXTURE_2D && prev_texture_2d_binding_ != 0) {
        glBindTexture(GL_TEXTURE_2D, prev_texture_2d_binding_);
        gl::check_error();
    }
}

void texture_t::upload(const image_t& image) {
    upload_pixels(image.width(), image.height(), reinterpret_cast<const uint32_t*>(image.data()));
}

void texture_t::upload_pixels(uint32_t width, uint32_t height, const uint32_t* pixels_rgba32) {
    begin_texture_setup(handle_, gl_texture_target_);

    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsRgba32);
    glTexImage2D(gl_texture_target_, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_rgba32);
    gl::check_error();

    end_texture_setup(gl_texture_target_);
}

void texture_t::uploadAlpha8(uint32_t width, uint32_t height, const uint8_t* alphaMap) {
    type_ = texture_type::alpha8;

    begin_texture_setup(handle_, gl_texture_target_);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    graphics::gl::check_error();

    auto textureFormat = GL_INTENSITY;
    auto internalFormat = GL_LUMINANCE;

    if (getContextType() == GraphicsContextType::OpenGL_ES_3) {
        textureFormat = GL_R8;
        internalFormat = GL_RED;
    }
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsRgba32);
    glTexImage2D(gl_texture_target_, 0, textureFormat, width, height, 0, internalFormat, GL_UNSIGNED_BYTE, alphaMap);
    gl::check_error();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    graphics::gl::check_error();

    if (getContextType() == GraphicsContextType::OpenGL_ES_3) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);
    }

    end_texture_setup(gl_texture_target_);
}

void texture_t::uploadSubAlpha8(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint8_t* alphaMap) {
    begin_texture_setup(handle_, gl_texture_target_);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    graphics::gl::check_error();

    auto textureFormat = GL_INTENSITY;
    auto internalFormat = GL_LUMINANCE;

    if (getContextType() == GraphicsContextType::OpenGL_ES_3) {
        textureFormat = GL_R8;
        internalFormat = GL_RED;
    }
    glTexSubImage2D(gl_texture_target_, 0, x, y, width, height, internalFormat, GL_UNSIGNED_BYTE, alphaMap);
    gl::check_error();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    graphics::gl::check_error();

    if (getContextType() == GraphicsContextType::OpenGL_ES_3) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);
    }

    glGenerateMipmap(gl_texture_target_);
    graphics::gl::check_error();
#if !defined(__ANDROID__)
    glTexParameterf(gl_texture_target_, GL_TEXTURE_LOD_BIAS, -0.7);
#endif
    end_texture_setup(gl_texture_target_, GL_LINEAR_MIPMAP_LINEAR);
}

void texture_t::bind(int unit) const {
    GL_CHECK(glActiveTexture(GL_TEXTURE0 + (GLenum) unit));
    GL_CHECK(glBindTexture(gl_texture_target_, handle_));

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
        case texture_type::alpha8:
            if (getContextType() == GraphicsContextType::OpenGL_ES_3) {
                texture_format = GL_R8;
                internal_format = GL_RED;
                pixel_type = GL_UNSIGNED_BYTE;
            } else {
                texture_format = GL_INTENSITY;
                internal_format = GL_LUMINANCE;
                pixel_type = GL_UNSIGNED_BYTE;
            }
            break;
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
    gl::check_error();

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
        gl::check_error();
    }
    end_texture_setup(gl_texture_target_);
}

}
