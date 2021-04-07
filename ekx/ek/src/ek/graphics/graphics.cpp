#include "graphics.hpp"
#include <ek/util/logger.hpp>
#include <ek/math/box.hpp>
#include <ek/Allocator.hpp>
#include <ek/assert.hpp>
#include <ek/imaging/image.hpp>
#include <ek/app/app.hpp>

static ek::ProxyAllocator* gHeapSokolGfx = nullptr;

#define SOKOL_GFX_IMPL

#define SOKOL_ASSERT(x) EK_ASSERT(x)
#define SOKOL_LOG(s) EK_INFO("SG: %s", s);

#define SOKOL_MALLOC(sz) (::gHeapSokolGfx->alloc(static_cast<uint32_t>(sz), static_cast<uint32_t>(sizeof(void*))))
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
    sg_range range;
    range.ptr = data;
    range.size = dataSize;
    sg_update_buffer(buffer, range);
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

void initialize(sg_context_desc* customContext) {
    gHeapSokolGfx = memory::stdAllocator.create<ProxyAllocator>("sokol_gfx");
    sg_desc desc{};
    desc.buffer_pool_size = 256;
    if(customContext != nullptr) {
        desc.context = *customContext;
    }
    else {
#if EK_MACOS || EK_IOS
        desc.context.metal.device = app::getMetalDevice();
        desc.context.metal.renderpass_descriptor_cb = app::getMetalRenderPass;
        desc.context.metal.drawable_cb = app::getMetalDrawable;
        desc.context.sample_count = 1;
        desc.context.color_format = SG_PIXELFORMAT_BGRA8;
        desc.context.depth_format = SG_PIXELFORMAT_DEPTH_STENCIL;
#endif
    }
    sg_setup(desc);
    auto backend = sg_query_backend();
    EK_INFO << "Sokol Backend: " << BackendToString[backend];
}

void shutdown() {
    sg_shutdown();
    memory::stdAllocator.destroy(gHeapSokolGfx);
}

/** Helpers **/

Texture* createTexture(const image_t& image, const char* label) {
    sg_image_desc desc{};
    desc.label = label;
    desc.type = SG_IMAGETYPE_2D;
    desc.width = (int) image.width();
    desc.height = (int) image.height();
    desc.usage = SG_USAGE_IMMUTABLE;
    desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    desc.min_filter = SG_FILTER_LINEAR;
    desc.mag_filter = SG_FILTER_LINEAR;
    desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    desc.data.subimage[0][0].ptr = image.data();
    desc.data.subimage[0][0].size = image.height() * image.stride();
    return new Texture(desc);
}

Texture* createTexture(ek::image_t* images[6], const char* label) {
    sg_image_desc desc{};
    desc.label = label;
    desc.type = SG_IMAGETYPE_CUBE;
    desc.width = (int) images[0]->width();
    desc.height = (int) images[0]->height();
    desc.usage = SG_USAGE_IMMUTABLE;
    desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    desc.min_filter = SG_FILTER_LINEAR;
    desc.mag_filter = SG_FILTER_LINEAR;
    desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;

    for (int i = 0; i < 6; ++i) {
        desc.data.subimage[i][0].ptr = images[i]->data();
        desc.data.subimage[i][0].size = images[i]->height() * images[i]->stride();
    }
    return new Texture(desc);
}

Texture* createRenderTarget(int width, int height, const char* label) {
    sg_image_desc desc{};
    desc.label = label;
    desc.type = SG_IMAGETYPE_2D;
    desc.render_target = true;
    desc.width = width;
    desc.height = height;
    desc.usage = SG_USAGE_IMMUTABLE;
    desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    desc.min_filter = SG_FILTER_LINEAR;
    desc.mag_filter = SG_FILTER_LINEAR;
    desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    return new Texture(desc);
}

}
