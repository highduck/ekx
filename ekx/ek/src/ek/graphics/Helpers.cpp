#include "Helpers.hpp"
#include "graphics.hpp"
#include <ek/imaging/image.hpp>


namespace ek::graphics {

Texture* createTexture(const image_t& image) {
    sg_image_desc desc{
            .type = SG_IMAGETYPE_2D,
            .width = (int) image.width(),
            .height = (int) image.height(),
            .usage = SG_USAGE_IMMUTABLE,
            .pixel_format = SG_PIXELFORMAT_RGBA8,
            .min_filter = SG_FILTER_LINEAR,
            .mag_filter = SG_FILTER_LINEAR,
            .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
            .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
    };
    desc.data.subimage[0][0].ptr = image.data();
    desc.data.subimage[0][0].size = image.height() * image.stride();
    return new Texture(desc);
}

Texture* createTexture(std::array<ek::image_t*, 6> images) {
    sg_image_desc desc{
            .type = SG_IMAGETYPE_CUBE,
            .width = (int) images[0]->width(),
            .height = (int) images[0]->height(),
            .usage = SG_USAGE_IMMUTABLE,
            .pixel_format = SG_PIXELFORMAT_RGBA8,
            .min_filter = SG_FILTER_LINEAR,
            .mag_filter = SG_FILTER_LINEAR,
            .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
            .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
    };

    for (int i = 0; i < 6; ++i) {
        desc.data.subimage[i][0].ptr = images[i]->data();
        desc.data.subimage[i][0].size = images[i]->height() * images[i]->stride();
    }
    return new Texture(desc);
}

Texture* createRenderTarget(int width, int height) {
    sg_image_desc desc{
            .type = SG_IMAGETYPE_2D,
            .render_target = true,
            .width = width,
            .height = height,
            .usage = SG_USAGE_IMMUTABLE,
            .pixel_format = SG_PIXELFORMAT_RGBA8,
            .min_filter = SG_FILTER_LINEAR,
            .mag_filter = SG_FILTER_LINEAR,
            .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
            .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
    };
    return new Texture(desc);
}

}
