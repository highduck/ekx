#pragma once

namespace texloader {

struct SubImage {
    void* data;
    int width;
    int height;
    int stride;
};

struct ImageData {
    bool isCubeMap = false;
    SubImage subImages[6];
    int subImagesCount = 0;
};

void setupImageDesc(const ImageData& data, sg_image_desc* sgDesc) {
    sgDesc.label = label;
    sgDesc.type = data.isCubeMap ? SG_IMAGETYPE_CUBE : SG_IMAGETYPE_2D;
    sgDesc.width = data.subImages[0].width;
    sgDesc.height = data.subImages[0].height;
    sgDesc.usage = SG_USAGE_IMMUTABLE;
    sgDesc.pixel_format = SG_PIXELFORMAT_RGBA8;
    sgDesc.min_filter = SG_FILTER_LINEAR;
    sgDesc.mag_filter = SG_FILTER_LINEAR;
    sgDesc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    sgDesc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    for (int i = 0; i < data.subImagesCount; ++i) {
        const auto& subImage = data.subImages[i];
        sgDesc.data.subimage[i][0].ptr = subImage.data;
        sgDesc.data.subimage[i][0].size = subImage.height * subImage.stride;
    }
}

}