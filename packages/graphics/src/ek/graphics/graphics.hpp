#pragma once

#include <ek/gfx.h>
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

    explicit Texture(sg_image image);

    ~Texture();

    void update(const void* data, uint32_t size) const;
    bool getPixels(void* pixels) const;

    sg_image image{};

    static Texture* solid(int width, int height, uint32_t pixelColor);
    static Texture* renderTarget(int width, int height, const char* label = nullptr);
};

}

#include <ek/util/Type.hpp>

namespace ek {

EK_DECLARE_TYPE(graphics::Texture);
EK_DECLARE_TYPE(graphics::Shader);
EK_TYPE_INDEX(graphics::Texture, 1);
EK_TYPE_INDEX(graphics::Shader, 2);

}