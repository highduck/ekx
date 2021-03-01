#include "graphics.hpp"
#include <ek/util/logger.hpp>
#include <ek/math/box.hpp>
#include <ek/Allocator.hpp>
#include <ek/assert.hpp>

static ek::ProxyAllocator* gHeapSokolGfx = nullptr;

#define SOKOL_GFX_IMPL

#define SOKOL_ASSERT(x) EK_ASSERT(x)
#define SOKOL_LOG(s) EK_INFO("SG: %s", s);

#define SOKOL_MALLOC(sz) (::gHeapSokolGfx->alloc(sz, sizeof(void*)))
#define SOKOL_FREE(p) (::gHeapSokolGfx->dealloc(p))

#include <sokol_gfx.h>

namespace ek::graphics {

/** buffer wrapper **/
Buffer::Buffer(BufferType type, const void* data, int dataSize) {
    sg_buffer_desc desc{
            .type = (sg_buffer_type) type,
            .usage = SG_USAGE_IMMUTABLE,
            .data = {
                    .ptr = data,
                    .size = (size_t)dataSize
            }
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
    if (buffer.id == 0) {
        abort();
    }
    size = maxSize;
}

Buffer::~Buffer() {
    sg_destroy_buffer(buffer);
}

void Buffer::update(const void* data, uint32_t dataSize) {
    size = dataSize;
    sg_update_buffer(buffer, sg_range{
        .ptr = data,
        .size = dataSize
    });
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
    sg_image_data imageData{};
    imageData.subimage[0][0].ptr = data;
    imageData.subimage[0][0].size = size;
    sg_update_image(image, imageData);
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
    desc.data.subimage[0][0] = {
            .ptr = pixels,
            .size = (size_t)count * 4
    };

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

void initialize() {
    gHeapSokolGfx = memory::stdAllocator.create<ProxyAllocator>("sokol_gfx");
    sg_desc desc{};
    desc.buffer_pool_size = 256;
    sg_setup(desc);
    auto backend = sg_query_backend();
    EK_INFO << "Sokol Backend: " << BackendToString[backend];
}

void shutdown() {
    sg_shutdown();
    memory::stdAllocator.destroy(gHeapSokolGfx);
}

}
