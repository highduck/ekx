#pragma once

#include "TextureLoader.hpp"
#include <ek/imaging/image.hpp>
#include <cstdlib>

// implement decode + stbi
#include <ek/imaging/Image_Decode.hpp>

#if defined(__APPLE__)

#include <TargetConditionals.h>
#include <Foundation/Foundation.h>

#endif

namespace ek {

inline int loadBytes(const char* basePath, const char* url, void** outBuffer, size_t* outSize) {
    char pathBuffer[1024] = "\0";
    if (basePath) {
        const size_t len = strlen(basePath);
        strcat(pathBuffer, basePath);
        if (len > 0 && pathBuffer[len - 1] != '/') {
            pathBuffer[len] = '/';
            pathBuffer[len + 1] = 0;
        }
    }
    strcat(pathBuffer, url);
#if defined(__APPLE__)
    NSString* asset_path = [NSString stringWithUTF8String:pathBuffer];
    NSString* bundle_path = [[NSBundle mainBundle] pathForResource:asset_path ofType:nil];
    const char* file_path = [bundle_path cStringUsingEncoding: NSASCIIStringEncoding];
#else
    const char* file_path = pathBuffer;
#endif

    auto* stream = fopen(file_path, "rb");
    if (!stream) {
        return 1;
    }
    fseek(stream, 0, SEEK_END);
    auto size = static_cast<size_t>(ftell(stream));
    *outSize = size;
    *outBuffer = malloc(size);
    fseek(stream, 0, SEEK_SET);

    fread(*outBuffer, size, 1u, stream);

    bool success = ferror(stream) == 0;
    fclose(stream);

    if (success) {
        return 0;
    }
    free(*outBuffer);
    *outBuffer = nullptr;
    *outSize = 0;
    return 2;
}

inline void setupImageDesc(const ImageData& data, sg_image_desc* sgDesc) {
    //sgDesc->label = label;
    sgDesc->type = data.isCubeMap ? SG_IMAGETYPE_CUBE : SG_IMAGETYPE_2D;
    sgDesc->width = data.width;
    sgDesc->height = data.height;
    sgDesc->usage = SG_USAGE_IMMUTABLE;
    sgDesc->pixel_format = SG_PIXELFORMAT_RGBA8;
    sgDesc->min_filter = SG_FILTER_LINEAR;
    sgDesc->mag_filter = SG_FILTER_LINEAR;
    sgDesc->wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    sgDesc->wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    for (int i = 0; i < data.subImagesCount; ++i) {
        const auto& subImage = data.subImages[i];
        sgDesc->data.subimage[i][0].ptr = subImage.data;
        sgDesc->data.subimage[i][0].size = subImage.height * subImage.stride;
    }
}

void TextureLoader::load() {
    loading = true;
    status = 0;
    progress = 0.0f;
    imageData.isCubeMap = isCubeMap;
    imageData.subImagesCount = imagesToLoad;
}

void TextureLoader::update() {
    if (imagesToLoad <= 0 || !loading) {
        return;
    }
    if (imagesStartLoading < imagesToLoad) {
        const auto idx = imagesStartLoading++;
        EK_DEBUG("poll loading image #%d / %d", idx, imagesToLoad);
        void* buffer = nullptr;
        size_t bufferSize = 0;
        int result = loadBytes(basePath.c_str(), urls[idx].c_str(), &buffer, &bufferSize);
        if (result == 0) {
            ++imagesLoaded;
            progress = (float) imagesLoaded / (float) imagesToLoad;
            auto* img = decode_image_data(buffer, bufferSize, premultiplyAlpha);
            if (img) {
                imageData.subImages[idx].data = img->data();
                imageData.subImages[idx].width = (int) img->width();
                imageData.subImages[idx].height = (int) img->height();
                imageData.subImages[idx].stride = (int) img->stride();
                img->forget();
            }
            delete img;
            EK_DEBUG("texture #%d loaded %d of %d", idx, imagesLoaded, imagesToLoad);
        } else {
            EK_ASSERT(false);
        }
    } else if (imagesLoaded == imagesToLoad) {
        imageData.width = imageData.subImages[0].width;
        imageData.height = imageData.subImages[0].height;
        setupImageDesc(imageData, &desc);
        image = sg_make_image(&desc);
        for (int i = 0; i < IMAGES_MAX_COUNT; ++i) {
            auto* subImageData = imageData.subImages[i].data;
            free((void*) subImageData);
            imageData.subImages[i] = {};
        }
        progress = 1.0f;
        loading = false;
    }
}


}