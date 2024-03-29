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

namespace ek {
class image_t;
}

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

class Shader : private NoCopyAssign {
public:
    sg_shader shader;
    uint8_t numFSImages;

    explicit Shader(const sg_shader_desc* desc);

    ~Shader();
};

class Buffer : private NoCopyAssign {
public:
    Buffer(BufferType type, Usage usage, uint32_t maxSize);

    // immutable init
    Buffer(BufferType type, const void* data, uint32_t dataSize);

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
Texture* createTexture(const image_t& image, const char* label = nullptr);

Texture* createTexture(ek::image_t* images[6], const char* label = nullptr);

Texture* createRenderTarget(int width, int height, const char* label = nullptr);

//void load_texture_lazy(const char* path, std::function<void(Texture*)> callback);
//void load_texture_cube_lazy(const std::vector<std::string>& path_list, std::function<void(Texture*)> callback);

}