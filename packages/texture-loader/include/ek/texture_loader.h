#ifndef EK_TEXTURE_LOADER_H
#define EK_TEXTURE_LOADER_H

#include <sokol_gfx.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ek_texture_loader_sub_image {
    const void* data;
    int width;
    int height;
    int stride;
} ek_texture_loader_sub_image;

typedef struct ek_texture_loader_image_data {
    int width;
    int height;
    bool isCubeMap;
    ek_texture_loader_sub_image subImages[6];
    int subImagesCount;
} ek_texture_loader_image_data;

#define EK_TEXTURE_LOADER_IMAGES_MAX_COUNT 6
#define EK_TEXTURE_LOADER_PATH_MAX 1024

typedef struct ek_texture_loader_path {
    char path[EK_TEXTURE_LOADER_PATH_MAX];
} ek_texture_loader_path;

typedef struct ek_texture_loader {

    uint32_t formatMask;
    ek_texture_loader_path basePath;
    ek_texture_loader_path urls[EK_TEXTURE_LOADER_IMAGES_MAX_COUNT];
    int imagesToLoad;
    bool isCubeMap;
    bool premultiplyAlpha;

    float progress;
    int imagesLoaded;
    int imagesStartLoading;
    bool loading;
    // 0 is success, != 0 - errors
    int status;
    ek_texture_loader_image_data imageData;
    int handle;

    sg_image image; // SG_INVALID_ID = 0
    sg_image_desc desc;
} ek_texture_loader;

void ek_texture_loader_set_path(ek_texture_loader_path* dst, const char* src);

ek_texture_loader* ek_texture_loader_create(void);
void ek_texture_loader_destroy(ek_texture_loader* loader);

void ek_texture_loader_init(ek_texture_loader* loader);

void ek_texture_loader_load(ek_texture_loader* loader);

void ek_texture_loader_update(ek_texture_loader* loader);

#ifdef __cplusplus
}
#endif

#endif // EK_TEXTURE_LOADER_H
