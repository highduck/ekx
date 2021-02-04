#pragma once

#include <ek/util/detect_platform.hpp>

#if EK_IOS

#define SOKOL_GLES3

#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>

#elif EK_ANDROID

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

#elif EK_WEB

//#define SOKOL_GLES3

//#include <GLES3/gl3.h>

#define SOKOL_GLES2

#ifndef GL_EXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#elif EK_MACOS

#define SOKOL_GLCORE33

#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif

#include <OpenGL/gl3.h>

#endif

#ifdef EK_EDITOR
#define SOKOL_TRACE_HOOKS
#endif

#include <sokol_gfx.h>

#include <ek/util/common_macro.hpp>



// sokol gfx objects RAII wrappers with utilities
namespace ek::graphics {

enum class GraphicsContextType {
    OpenGL,
    GLES_2,
    GLES_3,
    WebGL,
    WebGL_2
};


enum class Usage {
    Immutable = SG_USAGE_IMMUTABLE,
    Dynamic = SG_USAGE_DYNAMIC,
    Stream = SG_USAGE_STREAM
};

enum class BufferType {
    VertexBuffer = SG_BUFFERTYPE_VERTEXBUFFER,
    IndexBuffer = SG_BUFFERTYPE_INDEXBUFFER
};

class Shader : private disable_copy_assign_t {
public:
    sg_shader shader;
    uint8_t numFSImages;

    explicit Shader(const sg_shader_desc* desc);

    ~Shader();
};

class Buffer : private disable_copy_assign_t {
public:
    Buffer(BufferType type, Usage usage, int maxSize);

    // immutable init
    Buffer(BufferType type, const void* data, int dataSize);

    ~Buffer();

    void update(const void* data, uint32_t dataSize);

    [[nodiscard]]
    int getSize() const {
        return size;
    }

    sg_buffer buffer;
private:
    int size;
};

class Texture : private disable_copy_assign_t {
public:
    explicit Texture(const sg_image_desc& desc);

    ~Texture();

    void update(const void* data, uint32_t size) const;

    sg_image image{};
    sg_image_desc desc{};

    static Texture* createSolid32(int width, int height, uint32_t pixelColor);
};

void init();

}