#pragma once

#include <sokol_gfx.h>
#include <ek/ds/String.hpp>

namespace ek {

struct SubImage {
    const void* data;
    int width;
    int height;
    int stride;
};

struct ImageData {
    int width;
    int height;
    bool isCubeMap = false;
    SubImage subImages[6];
    int subImagesCount = 0;
};

class TextureLoader {
public:
    inline static const int IMAGES_MAX_COUNT = 6;

    uint32_t formatMask = 1;
    String basePath;
    String urls[IMAGES_MAX_COUNT];
    int imagesToLoad = 0;
    bool isCubeMap = false;
    bool premultiplyAlpha = true;

    float progress = 0.0f;
    int imagesLoaded = 0;
    int imagesStartLoading = 0;
    bool loading = false;
    // 0 is success, != 0 - errors
    int status = 0;
    ImageData imageData{};
    int handle = 0;

    sg_image image{SG_INVALID_ID};
    sg_image_desc desc{};

    void load();

    void update();
};

}