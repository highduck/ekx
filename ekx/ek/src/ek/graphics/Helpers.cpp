#include "Helpers.hpp"
#include "graphics.hpp"
#include <ek/imaging/image.hpp>

namespace ek::graphics {

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
