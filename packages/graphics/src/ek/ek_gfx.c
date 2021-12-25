#include <ek/gfx.h>
#include <ek/assert.h>
#include <ek/log.h>
#include <ek/app.h>
#include "temp_res_man.h"

#define SOKOL_GFX_IMPL

#define SOKOL_ASSERT(x) EK_ASSERT(x)
#define SOKOL_LOG(msg) EK_DEBUG("sg: %s", msg);

EK_DISABLE_WARNING_BEGIN()
EK_DISABLE_WARNING("-Wc99-extensions")

#include <sokol_gfx.h>

EK_DISABLE_WARNING_END()

static void ek_gfx_log_backend() {
#ifndef NDEBUG
    static const char* backend_strings[] = {
            "SG_BACKEND_GLCORE33",
            "SG_BACKEND_GLES2",
            "SG_BACKEND_GLES3",
            "SG_BACKEND_D3D11",
            "SG_BACKEND_METAL_IOS",
            "SG_BACKEND_METAL_MACOS",
            "SG_BACKEND_METAL_SIMULATOR",
            "SG_BACKEND_WGPU",
            "SG_BACKEND_DUMMY",
            0
    };
    const int backend = (int) sg_query_backend();
    EK_ASSERT(backend >= 0);
    EK_ASSERT(backend < (sizeof(backend_strings) / sizeof(backend_strings[0])));
    EK_INFO("sg backend: %s", backend_strings[backend]);
#endif
}

void ek_gfx_setup(int max_draw_calls) {
    EK_DEBUG("gfx init");
    sg_desc desc = {0};
    // this size is 2x Draw Calls per frame (because of sokol internal double-buffering)
    if (max_draw_calls <= 0) {
        max_draw_calls = 128;
    }
    desc.buffer_pool_size = max_draw_calls << 1;
#if defined(__APPLE__)
    desc.context.metal.device = ek_app_mtl_device();
    desc.context.metal.renderpass_descriptor_cb = ek_app_mtl_render_pass;
    desc.context.metal.drawable_cb = ek_app_mtl_drawable;
    desc.context.sample_count = 1;
    desc.context.color_format = SG_PIXELFORMAT_BGRA8;
    desc.context.depth_format = SG_PIXELFORMAT_DEPTH_STENCIL;
#endif
    sg_setup(&desc);
    ek_gfx_log_backend();


    ek_image_reg_setup();
}

void ek_gfx_shutdown(void) {
    EK_DEBUG("gfx shutdown");
    sg_shutdown();
}

bool ek_gfx_read_pixels(sg_image image, void* pixels) {
#if TARGET_OS_OSX
// get the texture from the sokol internals here...
    _sg_image_t* img = _sg_lookup_image(&_sg.pools, image.id);
    __unsafe_unretained id<MTLTexture> tex = _sg_mtl_id(img->mtl.tex[img->cmn.active_slot]);
    sg_image_info info = sg_query_image_info(image);
    const int width = info.width;
    const int height = info.height;
    id < MTLTexture > temp_texture = 0;
    if (_sg.mtl.cmd_queue && tex) {
        const MTLPixelFormat format = [tex
        pixelFormat];
        MTLTextureDescriptor * textureDescriptor = [MTLTextureDescriptor
        texture2DDescriptorWithPixelFormat:
        format
        width:
        (width)
        height:
        (height)
        mipmapped:
        NO];

        textureDescriptor.storageMode = MTLStorageModeManaged;
        textureDescriptor.resourceOptions = MTLResourceStorageModeManaged;
        textureDescriptor.usage = MTLTextureUsageShaderRead + MTLTextureUsageShaderWrite;
        temp_texture = [_sg.mtl.device
        newTextureWithDescriptor:
        textureDescriptor];
        if (temp_texture) {
            id<MTLCommandBuffer> cmdbuffer = [_sg.mtl.cmd_queue
            commandBuffer];
            id<MTLBlitCommandEncoder> blitcmd = [cmdbuffer
            blitCommandEncoder];

            [blitcmd
            copyFromTexture:
            tex
            sourceSlice:
            0
            sourceLevel:
            0
            sourceOrigin:
            MTLOriginMake(0, 0, 0)
            sourceSize:
            MTLSizeMake(width, height, 1)
            toTexture:
            temp_texture
            destinationSlice:
            0
            destinationLevel:
            0
            destinationOrigin:
            MTLOriginMake(0, 0, 0)];

            [blitcmd
            synchronizeTexture:
            temp_texture
            slice:
            0
            level:
            0];
            [blitcmd
            endEncoding];
            [cmdbuffer
            commit];
            [cmdbuffer
            waitUntilCompleted];
        }
    }
    if (temp_texture) {
        MTLRegion region = MTLRegionMake2D(0, 0, width, height);
        NSUInteger rowbyte = width * 4;
        [temp_texture
        getBytes:
        pixels
        bytesPerRow:
        rowbyte
        fromRegion:
        region
        mipmapLevel:
        0];
        return true;
    }
#else // OSX
    (void) image;
    (void) pixels;
#endif
    return false;
}

sg_image ek_gfx_make_color_image(int width, int height, uint32_t color) {
    sg_image_desc desc = {
            // defaults:
            //.type = SG_IMAGETYPE_2D,
            //.usage = SG_USAGE_IMMUTABLE,
            .width = width,
            .height = height,
            .pixel_format = SG_PIXELFORMAT_RGBA8,
    };
    int count = width * height;
    uint32_t * buffer = (uint32_t*) malloc(count * 4);
    for (int i = 0; i < count; ++i) {
        buffer[i] = color;
    }
    desc.data.subimage[0][0].ptr = buffer;
    desc.data.subimage[0][0].size = (size_t) count * 4;
    sg_image image = sg_make_image(&desc);
    free(buffer);
    return image;
}

sg_image ek_gfx_make_render_target(int width, int height, const char* label) {
    sg_image_desc desc = {
            // defaults:
            //.type = SG_IMAGETYPE_2D,
            //.usage = SG_USAGE_IMMUTABLE,

            .label = label,
            .render_target = true,
            .width = width,
            .height = height,

            //.pixel_format = SG_PIXELFORMAT_RGBA8,
            .min_filter = SG_FILTER_LINEAR,
            .mag_filter = SG_FILTER_LINEAR,
            .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
            .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
    };
    return sg_make_image(&desc);
}

void ek_gfx_update_image_0(sg_image image, void* data, size_t size) {
    sg_image_data image_data = {};
    image_data.subimage[0][0].ptr = data;
    image_data.subimage[0][0].size = size;
    sg_update_image(image, &image_data);
}
