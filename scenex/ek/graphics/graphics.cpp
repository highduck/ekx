#include "graphics.hpp"
#include <ek/util/logger.hpp>
#include <ek/math/box.hpp>

#define SOKOL_GFX_IMPL

#include <sokol_gfx.h>

namespace ek::graphics {

/** buffer wrapper **/
Buffer::Buffer(BufferType type, const void* data, int dataSize) {
    sg_buffer_desc desc{
            .size = dataSize,
            .type = (sg_buffer_type) type,
            .usage = SG_USAGE_IMMUTABLE,
            .content = data,
    };
    buffer = sg_make_buffer(&desc);
    size = dataSize;
}

Buffer::Buffer(BufferType type, Usage usage, int maxSize) {
    sg_buffer_desc desc{};
    desc.usage = (sg_usage) usage;
    desc.type = (sg_buffer_type) type;
    desc.size = maxSize;
    buffer = sg_make_buffer(&desc);
    size = maxSize;
}

Buffer::~Buffer() {
    sg_destroy_buffer(buffer);
}

void Buffer::update(const void* data, uint32_t dataSize) {
    size = dataSize;
    sg_update_buffer(buffer, data, dataSize);
//    sg_append_buffer(buffer, data, dataSize);
//    if (size > size_) {
//        sg_update_buffer(handle_, data, size);
//        size_ = size;
//    } else {
//        if (useDataOrphaning) {
//            GL_CHECK(glBufferData(type, size_, nullptr, usage));
//        }
//        GL_CHECK(glBufferSubData(type, 0, size, data));
//    }
}

/** shader wrapper **/
Shader::Shader(const sg_shader_desc* desc) {
    shader = sg_make_shader(desc);
    numFSImages = desc->fs.images[0].name != nullptr ? 1 : 0;
}

Shader::~Shader() {
    sg_destroy_shader(shader);
}

/** texture wrapper **/

Texture::Texture(const sg_image_desc& desc_) {
    desc = desc_;
    image = sg_make_image(desc_);
}

Texture::~Texture() {
    sg_destroy_image(image);
}

void Texture::update(const void* data, uint32_t size) const {
    sg_image_content content{};
    content.subimage[0][0].ptr = data;
    content.subimage[0][0].size = size;
    sg_update_image(image, content);
}

Texture* Texture::createSolid32(int width, int height, uint32_t pixelColor) {
    sg_image_desc desc{
            .type = SG_IMAGETYPE_2D,
            .width = width,
            .height = height,
            .usage = SG_USAGE_IMMUTABLE,
            .pixel_format = SG_PIXELFORMAT_RGBA8,
    };
    int count = width * height;
    auto* pixels = new uint32_t[count];
    for (int i = 0; i < count; ++i) {
        pixels[i] = pixelColor;
    }
    desc.content.subimage[0][0].ptr = pixels;
    desc.content.subimage[0][0].size = count * 4;

    auto* tex = new Texture(desc);
    delete[] pixels;
    return tex;
}

static std::string BackendToString[] = {
        "SG_BACKEND_GLCORE33",
        "SG_BACKEND_GLES2",
        "SG_BACKEND_GLES3",
        "SG_BACKEND_D3D11",
        "SG_BACKEND_METAL_IOS",
        "SG_BACKEND_METAL_MACOS",
        "SG_BACKEND_METAL_SIMULATOR",
        "SG_BACKEND_WGPU",
        "SG_BACKEND_DUMMY"
};

void init() {
    sg_desc desc{};
    sg_setup(desc);
    auto backend = sg_query_backend();
    EK_INFO << "Sokol Backend: " << BackendToString[backend];

    // TODO: move to app's create context
#ifdef __EMSCRIPTEN__
    //    glPixelStorei(GL_UNPACK_PREMULTIPLY_ALPHA_WEBGL, 1);
#endif
}

}
