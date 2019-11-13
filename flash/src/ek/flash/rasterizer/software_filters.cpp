#include "software_filters.h"

#include <ek/imaging/image.hpp>
#include <ek/imaging/drawing.hpp>

#include <algorithm>

using ek::spritepack::sprite_t;

namespace ek::flash {

void apply(const std::vector<filter_data_t>& filters, sprite_t& sprite, float scale) {
    const rect_i bounds = get_filtered_rect(sprite.source, filters);
    auto res = sprite;
    uint32_t width = bounds.width;
    uint32_t height = bounds.height;
    auto* dest = new image_t(width, height);
    const int2 dest_pos = sprite.source.position - bounds.position;
    image_t& img = *sprite.image;
    copy_pixels_normal(*dest, dest_pos, img, img.bounds<int>());

    for (auto& filter : filters) {
        apply(*dest, filter, bounds);
    }

    res.rc = bounds * (1.0f / scale);
    res.source = bounds;

    res.image = dest;
    // TODO: preserved pixels?
    delete sprite.image;
    sprite.image = nullptr;
    sprite = res;
}

}