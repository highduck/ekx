#pragma once

#include <ek/gfx.h>
#include <ek/util/NoCopyAssign.hpp>
#include <ek/util/Type.hpp>

namespace ek {

// sokol gfx objects RAII wrappers with utilities

class Shader : private NoCopyAssign {
public:
    sg_shader shader;
    uint8_t numFSImages;

    explicit Shader(const sg_shader_desc* desc);

    ~Shader();
};

//class Texture : private NoCopyAssign {
//public:
//    explicit Texture(const sg_image_desc& desc);
//
//    explicit Texture(sg_image image);
//
//    ~Texture();
//
//    void update(const void* data, uint32_t size) const;
//
//    bool getPixels(void* pixels) const;
//
//    sg_image image{};
//
//    static Texture* solid(int width, int height, uint32_t pixelColor);
//
//    static Texture* renderTarget(int width, int height, const char* label = nullptr);
//};

//EK_DECLARE_TYPE(Texture);
EK_DECLARE_TYPE(Shader);

}
