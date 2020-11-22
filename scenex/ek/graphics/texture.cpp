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

#ifndef GL_LUMINANCE_ALPHA
#define GL_LUMINANCE_ALPHA                0x190A
#endif

namespace ek::graphics {

texture_t::texture_t()
        : texture_t{false} {
}

texture_t::texture_t(bool is_cube_map)
        : is_cube_map_{is_cube_map} {
    setType(texture_type::color32);
    gl_texture_target_ = is_cube_map ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
    GL_CHECK(glGenTextures(1, &handle_));
}

texture_t::~texture_t() {
    assert(handle_ != 0);
    GL_CHECK(glDeleteTextures(1, &handle_));
}

static GLuint prev_texture_cube_map_binding_ = 0u;
static GLuint prev_texture_2d_binding_ = 0u;

void begin_texture_setup(GLuint texture_id, GLenum target_type) {
    GL_CHECK(glBindTexture(target_type, texture_id));
#ifdef EK_WEBGL
    // TODO: const / restore state
    // GL_CHECK(glPixelStorei(UNPACK_PREMULTIPLY_ALPHA_WEBGL, 0));
#endif
}

void end_texture_setup(GLenum target_type, GLenum minFilter = GL_LINEAR) {
    GL_CHECK(glTexParameteri(target_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(target_type, GL_TEXTURE_MIN_FILTER, minFilter));
    GL_CHECK(glTexParameteri(target_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(target_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
#ifndef EK_GLES2
    if (target_type == GL_TEXTURE_CUBE_MAP) {
        GL_CHECK(glTexParameteri(target_type, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
    }
#endif

    if (target_type == GL_TEXTURE_CUBE_MAP && prev_texture_cube_map_binding_ != 0) {
        GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, prev_texture_cube_map_binding_));
    } else if (target_type == GL_TEXTURE_2D && prev_texture_2d_binding_ != 0) {
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, prev_texture_2d_binding_));
    }
}

void texture_t::upload(const image_t& image) {
    upload_pixels(image.width(), image.height(), image.data());
}

void texture_t::upload_pixels(uint32_t width, uint32_t height, const uint8_t* data) {
    begin_texture_setup(handle_, gl_texture_target_);

    uint8_t* emulatedData = nullptr;
    if (type_ == texture_type::alpha8) {
        GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
        if (getContextType() == GraphicsContextType::OpenGL_ES_2) {
            emulatedData = new uint8_t[width * height * 2];
            unsigned len = width * height;
            uint8_t* ptr = emulatedData;
            for (unsigned i = 0; i < len; ++i) {
                *(ptr++) = data[i];
                *(ptr++) = data[i];
            }
            data = emulatedData;
        }
    }

    GL_CHECK(glTexImage2D(gl_texture_target_, 0, textureFormat, width, height,
                          0, internalFormat, pixelType, data));

    if (type_ == texture_type::alpha8) {
        GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
        if (getContextType() == GraphicsContextType::OpenGL_ES_3) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);
        }
    }

    if (mipmaps) {
        GL_CHECK(glGenerateMipmap(gl_texture_target_));
#if !defined(__ANDROID__)
        GL_CHECK(glTexParameterf(gl_texture_target_, GL_TEXTURE_LOD_BIAS, mipmapBias));
#endif
    }
    end_texture_setup(gl_texture_target_, mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

    delete[] emulatedData;
}

void texture_t::updateRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint8_t* data) {
    begin_texture_setup(handle_, gl_texture_target_);

    uint8_t* emulatedData = nullptr;
    if (type_ == texture_type::alpha8) {
        if (getContextType() == GraphicsContextType::OpenGL_ES_2) {
            emulatedData = new uint8_t[width * height * 2];
            unsigned len = width * height;
            uint8_t* ptr = emulatedData;
            for (unsigned i = 0; i < len; ++i) {
                *(ptr++) = data[i];
                *(ptr++) = data[i];
            }
            data = emulatedData;
        }
        GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    }

    GL_CHECK(glTexSubImage2D(gl_texture_target_, 0, x, y, width, height, internalFormat, pixelType, data));

    if (type_ == texture_type::alpha8) {
        GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
        if (getContextType() == GraphicsContextType::OpenGL_ES_3) {
            GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED));
            GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED));
            GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED));
        }
    }

    if (mipmaps) {
        GL_CHECK(glGenerateMipmap(gl_texture_target_));
#if !defined(__ANDROID__)
        GL_CHECK(glTexParameterf(gl_texture_target_, GL_TEXTURE_LOD_BIAS, mipmapBias));
#endif
    }
    end_texture_setup(gl_texture_target_, mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

    delete[] emulatedData;
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
    setType(type);

    begin_texture_setup(handle_, gl_texture_target_);

    if (is_cube_map_) {
        for (int i = 0; i < 6; ++i) {
            GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                  0, internalFormat, width, height,
                                  0, textureFormat, pixelType, nullptr));
        }
    } else {
        GL_CHECK(glTexImage2D(gl_texture_target_, 0, internalFormat, width, height,
                              0, textureFormat, pixelType, nullptr));
    }

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

void texture_t::setType(texture_type type) {
    type_ = type;

    internalFormat = GL_RGBA;
    textureFormat = GL_RGBA;
    pixelType = GL_UNSIGNED_BYTE;

    switch (type_) {
        case texture_type::alpha8:
            if (getContextType() == GraphicsContextType::OpenGL_ES_3) {
                textureFormat = GL_R8;
                internalFormat = GL_RED;
                pixelType = GL_UNSIGNED_BYTE;
            } else if (getContextType() == GraphicsContextType::OpenGL_ES_2) {
                textureFormat = GL_LUMINANCE_ALPHA;
                internalFormat = GL_LUMINANCE_ALPHA;
                pixelType = GL_UNSIGNED_BYTE;
            } else {
                textureFormat = GL_INTENSITY;
                internalFormat = GL_LUMINANCE;
                pixelType = GL_UNSIGNED_BYTE;
            }
            break;
        case texture_type::depth16:
            textureFormat = GL_DEPTH_COMPONENT;
            internalFormat = GL_DEPTH_COMPONENT16;
            pixelType = GL_UNSIGNED_SHORT;
            break;
        case texture_type::depth24:
            textureFormat = GL_DEPTH_COMPONENT;
#ifdef EK_GLES2
            internalFormat = GL_DEPTH_COMPONENT16; // ???
            internalFormat = GL_DEPTH_COMPONENT;
            pixelType = GL_UNSIGNED_SHORT;
#else
            internalFormat = GL_DEPTH_COMPONENT24;
            pixelType = GL_UNSIGNED_INT;
#endif
            break;
        default:
            break;
    }
}

void texture_t::setMipMaps(bool enabled, float bias) {
    mipmaps = enabled;
    mipmapBias = bias;
}

}
