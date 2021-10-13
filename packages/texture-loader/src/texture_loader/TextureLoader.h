#pragma once

#include <sokol_gfx.h>

#if defined(__EMSCRIPTEN__)

#include <emscripten.h>

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

extern int texloader_create(const char* pBasePath, const char** pUrls, int numImages, int cubeMap, int pma);
extern int texloader_get(int id, texloader_state* pState);
extern int texloader_destroy(int id);

}
#else

#include <ek/util/Path.hpp>
#include <ek/app/res.hpp>
#include <ek/imaging/image.hpp>

#endif

namespace ek {

class TextureLoader {
public:
    inline static const int IMAGES_MAX_COUNT = 6;

    std::string basePath;
    std::string urls[IMAGES_MAX_COUNT];
    int imagesToLoad = 0;
    bool isCubeMap = false;
    bool premultiplyAlpha = true;

    float progress = 0.0f;
    bool loading = false;
    // 0 is success, != 0 - errors
    int status = 0;
    int handle = 0;

    sg_image image = nullptr;
    sg_image_desc desc{};

    void load();

    void update();
};

#if defined(__EMSCRIPTEN__)

void TextureLoader::load() {
    loading = true;
    status = 0;
    progress = 0.0f;
    const char* urls_[IMAGES_MAX_COUNT]{};
    for (int i = 0; i < IMAGES_MAX_COUNT; ++i) {
        urls_[i] = urls[i].c_str();
    }
    int flags = 0;
    if(premultiplyAlpha) flags|= 1;
    if(isCubeMap) flags |= 2;
    handle = texloader_create(urls_, imagesToLoad, basePath.c_str(), nullptr, flags);
}

void TextureLoader::update() {
    if (imagesToLoad <= 0 || !loading) {
        return;
    }
    texloader_state state{};
    texloader_get(handle, &state);
    progress = 0.01f * (float)state.progress;
    if (state.status == 2) {
        sg_image_desc desc{};
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
        texture = new Texture(desc);

        texloader_destroy(handle);
        handle = 0;
        progress = 1.0f;
        loading = false;
        status = 0;
    }
}

#else

void TextureLoader::load() {
    loading = true;
    status = 0;
    progress = 0.0f;
}

void TextureLoader::update() {
    if (imagesToLoad <= 0 || !loading) {
        return;
    }
    if (imagesStartLoading < imagesToLoad) {
        const path_t aBasePath{basePath};
        const auto idx = imagesStartLoading++;
        EK_DEBUG_F("poll loading image #%d / %d", idx, imagesToLoad);
        get_resource_content_async(
                (aBasePath / urls[idx]).c_str(),
                [this, _idx = idx](auto buffer) {
                    ++imagesLoaded;
                    progress = (float) imagesLoaded / (float) imagesToLoad;
                    images[_idx] = decode_image_data(buffer.data(), buffer.size(), premultiplyAlpha);
                    EK_DEBUG_F("texture #%d loaded %d of %d", _idx, imagesLoaded, imagesToLoad);
                }
        );
    } else if (imagesLoaded == imagesToLoad) {
        if (isCubeMap) {
            EK_DEBUG("Cube map images loaded, creating cube texture and cleaning up");
            texture = graphics::createTexture(images);
        } else {
            if (images[0]) {
                texture = graphics::createTexture(*images[0]);
            } else {
                status = 1;
            }
        }
        for (int i = 0; i < IMAGES_MAX_COUNT; ++i) {
            if (images[i]) {
                delete images[i];
                images[i] = nullptr;
            }
        }
        progress = 1.0f;
        loading = false;
    }
}

#endif

}