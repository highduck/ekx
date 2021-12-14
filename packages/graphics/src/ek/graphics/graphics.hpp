#pragma once

#if defined(__ANDROID__)

//#define SOKOL_GLES3

//#include <GLES3/gl3.h>
//#include <GLES3/gl3ext.h>

#define SOKOL_GLES2

//#include <EGL/egl.h>

#ifndef GL_EXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#undef GL_ANGLE_instanced_arrays
#undef GL_EXT_draw_instanced

#elif defined(__EMSCRIPTEN__)

//#define SOKOL_GLES3

//#include <GLES3/gl3.h>

#define SOKOL_GLES2

#ifndef GL_EXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#elif defined (__APPLE__)

#define SOKOL_METAL

#elif defined(__linux__)

#define SOKOL_GLCORE33

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

#elif defined(_WIN32) || defined(_WIN64)

#define SOKOL_D3D11

#endif

#include <sokol_gfx.h>

#include <ek/util/NoCopyAssign.hpp>

// sokol gfx objects RAII wrappers with utilities
namespace ek::graphics {

class Shader : private NoCopyAssign {
public:
    sg_shader shader;
    uint8_t numFSImages;

    explicit Shader(const sg_shader_desc* desc);

    ~Shader();
};

class Buffer : private NoCopyAssign {
public:
    Buffer(sg_buffer_type type, sg_usage usage, uint32_t maxSize);

    // immutable init
    Buffer(sg_buffer_type type, const void* data, uint32_t dataSize);

    ~Buffer();

    void update(const void* data, uint32_t dataSize);

    [[nodiscard]]
    inline uint32_t getSize() const { return size; }

    sg_buffer buffer;
private:
    uint32_t size;
};

class Texture : private NoCopyAssign {
public:
    explicit Texture(const sg_image_desc& desc);

    Texture(sg_image image, const sg_image_desc& desc);

    ~Texture();

    void update(const void* data, uint32_t size) const;
    bool getPixels(void* pixels) const;

    sg_image image{};
    sg_image_desc desc{};

    static Texture* createSolid32(int width, int height, uint32_t pixelColor);
};

void initialize(int maxDrawCalls = 128);

void shutdown();

/*** Helpers ***/
Texture* createRenderTarget(int width, int height, const char* label = nullptr);

}

#include <ek/util/Type.hpp>

namespace ek {

EK_DECLARE_TYPE(graphics::Texture);
EK_DECLARE_TYPE(graphics::Shader);
EK_TYPE_INDEX(graphics::Texture, 1);
EK_TYPE_INDEX(graphics::Shader, 2);

}