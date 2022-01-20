#pragma once

#include "types.h"
#include <pugixml.hpp>

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_ONLY_BMP
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif

#include <stb/stb_image_write.h>

namespace bmfont_export {

void save(ImageCollection& images, const char* output) {
    pugi::xml_document doc{};
    auto nodeAtlas = doc.append_child("images");
    int idx = 0;
    char path[1024];

    for (auto& resolution: images.resolutions) {
        auto nodeResolution = nodeAtlas.append_child("resolution");
        for (auto& image: resolution.images) {
            if (image.bitmap.pixels) {
                auto bitmap = image.bitmap;
                auto nodeSprite = nodeResolution.append_child("image");
                snprintf(path, 1024, "%s/%d.bmp", output, idx++);
                // require RGBA non-premultiplied alpha
                ek_bitmap_unpremultiply(&bitmap);
                stbi_write_bmp(path, bitmap.w, bitmap.h, 4, bitmap.pixels);

                nodeSprite.append_attribute("path").set_value(path);
                nodeSprite.append_attribute("name").set_value(image.name.c_str());
                nodeSprite.append_attribute("x").set_value(image.rc.x);
                nodeSprite.append_attribute("y").set_value(image.rc.y);
                nodeSprite.append_attribute("w").set_value(image.rc.w);
                nodeSprite.append_attribute("h").set_value(image.rc.h);
                nodeSprite.append_attribute("p").set_value(image.padding);
            }
        }
    }
    snprintf(path, 1024, "%s/_images.xml", output);
    doc.save_file(path);
}

}