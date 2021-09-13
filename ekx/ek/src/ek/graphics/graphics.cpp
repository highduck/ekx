#include "graphics.hpp"
#include <ek/Allocator.hpp>
#include <ek/debug.hpp>
#include <ek/assert.hpp>
#include <ek/imaging/image.hpp>
#include <ek/math/box.hpp>
#include <ek/app/app.hpp>
#include <ek/app/Platform.h>

static ek::ProxyAllocator* gHeapSokolGfx = nullptr;

#define SOKOL_GFX_IMPL

#define SOKOL_ASSERT(x) EK_ASSERT(x)
#define SOKOL_LOG(s) EK_INFO("SG: %s", s);

#define SOKOL_MALLOC(sz) (::gHeapSokolGfx->alloc(static_cast<uint32_t>(sz), static_cast<uint32_t>(sizeof(void*))))
#define SOKOL_FREE(p) (::gHeapSokolGfx->dealloc(p))

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc99-extensions"
#include <sokol_gfx.h>
#pragma clang diagnostic pop

namespace ek::graphics {

/** buffer wrapper **/
Buffer::Buffer(BufferType type, const void* data, uint32_t dataSize) {
    sg_buffer_desc desc{};
    desc.type = (sg_buffer_type) type;
    desc.usage = SG_USAGE_IMMUTABLE;
    desc.data.ptr = data;
    desc.data.size = (size_t)dataSize;
    buffer = sg_make_buffer(&desc);
    size = dataSize;
}

Buffer::Buffer(BufferType type, Usage usage, uint32_t maxSize) {
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
    sg_image_desc desc{};
    desc.type = SG_IMAGETYPE_2D;
    desc.width = width;
    desc.height = height;
    desc.usage = SG_USAGE_IMMUTABLE;
    desc.pixel_format = SG_PIXELFORMAT_RGBA8;

    int count = width * height;
    auto* pixels = new uint32_t[count];
    for (int i = 0; i < count; ++i) {
        pixels[i] = pixelColor;
    }
    desc.data.subimage[0][0].ptr = pixels;
    desc.data.subimage[0][0].size = (size_t)count * 4;
    auto* tex = new Texture(desc);
    delete[] pixels;
    return tex;
}

bool Texture::getPixels(void* pixels) const {
    (void)pixels;
#if EK_MACOS
    // get the texture from the sokol internals here...
    _sg_image_t* img = _sg_lookup_image(&_sg.pools, image.id);
    __unsafe_unretained id<MTLTexture> tex = _sg_mtl_id(img->mtl.tex[img->cmn.active_slot]);
    const auto width = desc.width;
    const auto height = desc.height;
    id<MTLTexture> temp_texture = 0;
    if (_sg.mtl.cmd_queue && tex) {
        const MTLPixelFormat format = [tex pixelFormat];
        MTLTextureDescriptor* textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format width:(width) height:(height) mipmapped:NO];

        textureDescriptor.storageMode = MTLStorageModeManaged;
        textureDescriptor.resourceOptions = MTLResourceStorageModeManaged;
        textureDescriptor.usage = MTLTextureUsageShaderRead + MTLTextureUsageShaderWrite;
        temp_texture = [_sg.mtl.device newTextureWithDescriptor:textureDescriptor];
        if (temp_texture) {
            id<MTLCommandBuffer> cmdbuffer = [_sg.mtl.cmd_queue commandBuffer];
            id<MTLBlitCommandEncoder> blitcmd = [cmdbuffer blitCommandEncoder];

            [blitcmd copyFromTexture:tex
                sourceSlice:0 sourceLevel:0 sourceOrigin:MTLOriginMake(0,0,0)
                sourceSize:MTLSizeMake(width,height,1)
                toTexture:temp_texture
                destinationSlice:0 destinationLevel:0
                destinationOrigin:MTLOriginMake(0,0,0)];

            [blitcmd synchronizeTexture:temp_texture slice:0 level:0];
            [blitcmd endEncoding];

            [cmdbuffer commit];
            [cmdbuffer waitUntilCompleted];
        }
    }
    if (temp_texture) {
        MTLRegion region = MTLRegionMake2D(0, 0, width, height);
        NSUInteger rowbyte = width * 4;
        [temp_texture getBytes:pixels bytesPerRow:rowbyte fromRegion:region mipmapLevel:0];
        return true;
    }
#endif // defined(SOKOL_METAL) && defined(EK_DEV_TOOLS)
    return false;
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

void initialize(int maxDrawCalls) {
    EK_TRACE << "graphics initialize";
    gHeapSokolGfx = memory::stdAllocator.create<ProxyAllocator>("sokol_gfx");
    sg_desc desc{};
    // this size is 2x Draw Calls per frame (because of sokol internal double-buffering)
    desc.buffer_pool_size = maxDrawCalls << 1;
#if EK_MACOS || EK_IOS
    desc.context.metal.device = app::getMetalDevice();
    desc.context.metal.renderpass_descriptor_cb = app::getMetalRenderPass;
    desc.context.metal.drawable_cb = app::getMetalDrawable;
    desc.context.sample_count = 1;
    desc.context.color_format = SG_PIXELFORMAT_BGRA8;
    desc.context.depth_format = SG_PIXELFORMAT_DEPTH_STENCIL;
#endif
    sg_setup(desc);
    auto backend = sg_query_backend();
    EK_INFO << "Sokol Backend: " << BackendToString[backend];
}

void shutdown() {
    EK_TRACE << "graphics shutdown";
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
    //desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    desc.min_filter = SG_FILTER_LINEAR;
    desc.mag_filter = SG_FILTER_LINEAR;
    desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    return new Texture(desc);
}

}
