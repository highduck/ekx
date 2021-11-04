#pragma once

#include "TextureLoader.hpp"
#include <emscripten.h>
#include <GLES2/gl2.h>

extern "C" {

typedef struct {
    // 0...100
    int progress;
    int width;
    int height;
    // 2 is completed
    int status;
    int texture;
} texloader_state;

extern int texloader_create(const char** pUrls, int numImages, const char* pBasePath, const char* pVersion, int flags, int formatMask);
extern int texloader_get(int id, texloader_state* pState);
extern int texloader_destroy(int id);

}

namespace ek {

void TextureLoader::load() {
    loading = true;
    status = 0;
    progress = 0.0f;
    const char* urls_[IMAGES_MAX_COUNT]{};
    for (int i = 0; i < IMAGES_MAX_COUNT; ++i) {
        urls_[i] = urls[i].c_str();
    }
    int flags = 0;
    if (premultiplyAlpha) flags |= 1;
    if (isCubeMap) flags |= 2;
    handle = texloader_create(urls_, imagesToLoad, basePath.c_str(), nullptr, flags, formatMask);
}

void TextureLoader::update() {
    if (imagesToLoad <= 0 || !loading) {
        return;
    }
    texloader_state state{};
    texloader_get(handle, &state);
    progress = 0.01f * (float) state.progress;
    if (state.status == 2) {
        //desc.label = label;
        desc.type = isCubeMap ? SG_IMAGETYPE_CUBE : SG_IMAGETYPE_2D;
        desc.width = state.width;
        desc.height = state.height;
        desc.usage = SG_USAGE_IMMUTABLE;
        desc.pixel_format = SG_PIXELFORMAT_RGBA8;
        desc.min_filter = SG_FILTER_LINEAR;
        desc.mag_filter = SG_FILTER_LINEAR;
        desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
        desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
        desc.gl_textures[0] = state.texture;
        desc.gl_texture_target = isCubeMap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
        image = sg_make_image(&desc);

        texloader_destroy(handle);
        handle = 0;
        progress = 1.0f;
        loading = false;
        status = 0;
    }
}

}