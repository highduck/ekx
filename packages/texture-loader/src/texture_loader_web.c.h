#include <ek/texture_loader.h>
#include <emscripten.h>
#include <GLES2/gl2.h>

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

void ek_texture_loader_load(ek_texture_loader* loader) {
    loader->loading = true;
    loader->status = 0;
    loader->progress = 0.0f;
    const char* urls_[EK_TEXTURE_LOADER_IMAGES_MAX_COUNT];
    for (int i = 0; i < EK_TEXTURE_LOADER_IMAGES_MAX_COUNT; ++i) {
        urls_[i] = loader->urls[i].path;
    }
    int flags = 0;
    if (loader->premultiplyAlpha) flags |= 1;
    if (loader->isCubeMap) flags |= 2;
    loader->handle = texloader_create(urls_, loader->imagesToLoad, loader->basePath.path, 0, flags, (int)loader->formatMask);
}

void ek_texture_loader_update(ek_texture_loader* loader) {
    if (loader->imagesToLoad <= 0 || !loader->loading) {
        return;
    }
    texloader_state state = {0};
    texloader_get(loader->handle, &state);
    loader->progress = 0.01f * (float) state.progress;
    if (state.status == 2) {
        //desc.label = label;
        loader->desc.type = loader->isCubeMap ? SG_IMAGETYPE_CUBE : SG_IMAGETYPE_2D;
        loader->desc.width = state.width;
        loader->desc.height = state.height;
        loader->desc.usage = SG_USAGE_IMMUTABLE;
        loader->desc.pixel_format = SG_PIXELFORMAT_RGBA8;
        loader->desc.min_filter = SG_FILTER_LINEAR;
        loader->desc.mag_filter = SG_FILTER_LINEAR;
        loader->desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
        loader->desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
        loader->desc.gl_textures[0] = state.texture;
        loader->desc.gl_texture_target = loader->isCubeMap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
        loader->image = sg_make_image(&loader->desc);

        texloader_destroy(loader->handle);
        loader->handle = 0;
        loader->progress = 1.0f;
        loader->loading = false;
        loader->status = 0;
    }
}
