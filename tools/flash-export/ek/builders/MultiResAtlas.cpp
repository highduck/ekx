#include "MultiResAtlas.hpp"
#include <pugixml.hpp>
#include <ek/time/Stopwatch.hpp>
#include <thread>
#include <ek/debug.hpp>
#include <ek/util/Path.hpp>
#include <ek/math/max_rects.hpp>
#include <ek/imaging/drawing.hpp>
#include <ek/imaging/ImageSubSystem.hpp>
#include <ek/Allocator.hpp>
#include <ek/assert.hpp>

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#define STBIW_ASSERT(e)   EK_ASSERT(e)

// TODO: enable allocation tracking for multi-threaded tasks
//#define STBIW_ALLOCATOR  ::ek::imaging::allocator
// we are using multi-threading there!
#define STBIW_ALLOCATOR  ::ek::memory::systemAllocator

#define STBIW_MALLOC(size)                           STBIW_ALLOCATOR.alloc(size, sizeof(void**))
#define STBIW_REALLOC_SIZED(ptr, oldSize, newSize)   STBIW_ALLOCATOR.reallocate(ptr, oldSize, newSize, sizeof(void**))
#define STBIW_FREE(ptr)                              STBIW_ALLOCATOR.dealloc(ptr)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-compare"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#include <stb/stb_image_write.h>
#pragma clang diagnostic pop

#endif

namespace ek {

void MultiResAtlasSettings::readFromXML(const pugi::xml_node& node) {

    name = node.attribute("name").as_string();
    for (auto& resolution_node: node.children("resolution")) {
        Resolution res{};
        res.scale = resolution_node.attribute("scale").as_float(res.scale);
        res.max_size.x = resolution_node.attribute("max_width").as_float(res.max_size.x);
        res.max_size.y = resolution_node.attribute("max_height").as_float(res.max_size.y);
        resolutions.push_back(res);
    }
}

void MultiResAtlasSettings::writeToXML(pugi::xml_node& node) const {
    node.append_attribute("name").set_value(name.c_str());
    for (const auto& resolution: resolutions) {
        auto resolution_node = node.append_child("resolution");
        resolution_node.append_attribute("scale").set_value(resolution.scale);
        resolution_node.append_attribute("max_width").set_value(resolution.max_size.x);
        resolution_node.append_attribute("max_height").set_value(resolution.max_size.y);
    }
}


inline std::string get_atlas_suffix(float scale, int page_index = 0) {
    std::string suffix;
    if (page_index != 0) {
        suffix += "_" + std::to_string(page_index);
    }

    if (scale > 3) suffix += "@4x";
    else if (scale > 2) suffix += "@3x";
    else if (scale > 1) suffix += "@2x";
    return suffix;
}

void save_stream(const output_memory_stream& stream, const path_t& path) {
    auto h = fopen(path.c_str(), "wb");
    if (h) {
        fwrite(stream.data(), 1, stream.size(), h);
        fclose(h);
    } else {
        EK_WARN << "fopen error: " << path;
    }
}

void save_atlas_resolution(AtlasData& resolution, const std::string& name) {
    int page_index = 0;
    for (auto& page : resolution.pages) {
        assert(page.image != nullptr);
        page.image_path = name + get_atlas_suffix(resolution.resolution_scale, page_index) + ".png";
        saveImagePNG(*page.image, page.image_path);
        ++page_index;
    }

    output_memory_stream os{100};
    IO io{os};
    io(const_cast<AtlasData&>(resolution));
    save_stream(os, path_t{name + get_atlas_suffix(resolution.resolution_scale) + ".atlas"});
}

static void packAtlasThread(const std::string& name, AtlasData& resolution) {
    const std::string log_name = name + "-" + std::to_string(resolution.resolution_index);
    {
        EK_INFO("  - Begin Pack %s", log_name.c_str());
        Stopwatch timer{};
        resolution.pages = packSprites(resolution.sprites, resolution.max_size);
        EK_INFO("  - End Pack %s : %0.3f ms", log_name.c_str(), timer.readMillis());
    }
    {
        EK_INFO("  - Begin Save %s", log_name.c_str());
        Stopwatch timer{};
        save_atlas_resolution(resolution, name);
        EK_INFO("  - End Save %s : %0.3f ms", log_name.c_str(), timer.readMillis());
    }
}

void MultiResAtlasData::packAndSaveMultiThreaded() {
    Stopwatch timer{};

    std::vector<std::thread> threads;
    threads.reserve(resolutions.size());

    for (auto& resolution : resolutions) {
        threads.emplace_back(&packAtlasThread, name, std::ref(resolution));
    }

    for (auto& th : threads) {
        th.join();
    }

    EK_INFO("ATLAS BUILD %s : %0.3f ms", name.c_str(), timer.readMillis());
}

/*** Pack Sprites ***/


rect_i no_pack_padding(binpack::rect_t rect, int pad) {
    return {
            rect.x + pad,
            rect.y + pad,
            rect.width - 2 * pad,
            rect.height - 2 * pad
    };
}

rect_f calc_uv(rect_i source, float2 atlas_size, bool rotated) {
    if (rotated) {
        std::swap(source.width, source.height);
    }
    return source / atlas_size;
}


Array<AtlasPageData> packSprites(Array<SpriteData> sprites, const int2 max_size) {
    Array<AtlasPageData> pages;

//    timer timer{};
//    EK_DEBUG("Packing %lu sprites...", sprites.size());

    bool need_to_pack = true;
    while (need_to_pack) {
        binpack::packer_state_t packer_state{max_size.x, max_size.y};

        for (auto& sprite: sprites) {
            if (!sprite.is_packed()) {
                packer_state.add(
                        sprite.source.width,
                        sprite.source.height,
                        sprite.padding,
                        &sprite);
            }
        }

        need_to_pack = !packer_state.empty();
        if (need_to_pack) {
            pack_nodes(packer_state);

            AtlasPageData page{};
            page.size = {packer_state.canvas.width, packer_state.canvas.height};
            page.image = new image_t(page.size.x, page.size.y);
            const float2 atlas_page_size_f{
                    static_cast<float>(page.size.x),
                    static_cast<float>(page.size.y)
            };

            {
                //atlas_renderer_cairo renderer{*page.image};
                for (size_t i = 0; i < packer_state.rects.size(); ++i) {
                    if (!packer_state.is_packed(i)) {
                        continue;
                    }
                    auto& sprite = packer_state.get_user_data<SpriteData>(i);
                    sprite.enable(SpriteFlag::Packed);
                    if (packer_state.is_rotated(i)) {
                        sprite.enable(SpriteFlag::Rotated);
                    }

                    const rect_i packed_rect = no_pack_padding(packer_state.get_rect(i), sprite.padding);
                    if (sprite.is_rotated()) {
                        copy_pixels_ccw_90(*page.image, packed_rect.position, *sprite.image, sprite.source);
                    } else {
                        copy_pixels_normal(*page.image, packed_rect.position, *sprite.image, sprite.source);
                    }
                    delete sprite.image;
                    sprite.image = page.image;
                    sprite.source = packed_rect;
                    sprite.uv = calc_uv(packed_rect, atlas_page_size_f, sprite.is_rotated());

                    page.sprites.emplace_back(sprite);
                }
            }
            pages.emplace_back(std::move(page));
        }
    }

//    LOG_PERF("Packed %lu to %lu pages for %lf ms",
//             sprites.size(),
//             pages.size(),
//             get_elapsed_time(timer));

    return pages;
}

// TODO: atlas
// ++page_index;
// page.image_path = atlas.name + get_atlas_suffix(atlas.scale, page_index) + ".png";

/*** Save Image ***/

void saveImagePNG(const image_t& image, const std::string& path, bool alpha) {
    image_t img{image};
    // require RGBA non-premultiplied alpha
    undo_premultiply_image(img);

    if (alpha) {
        stbi_write_png(path.c_str(),
                       img.width(),
                       img.height(),
                       4,
                       img.data(),
                       4 * static_cast<int>(img.width()));
    } else {

        size_t pixels_count = img.width() * img.height();
        auto* buffer = (uint8_t*) imaging::allocator.alloc(pixels_count * 3, sizeof(void*));
        auto* buffer_rgb = buffer;
        auto* buffer_rgba = img.data();

        for (size_t i = 0; i < pixels_count; ++i) {
            buffer_rgb[0] = buffer_rgba[0];
            buffer_rgb[1] = buffer_rgba[1];
            buffer_rgb[2] = buffer_rgba[2];
            buffer_rgba += 4;
            buffer_rgb += 3;
        }

        stbi_write_png(path.c_str(),
                       img.width(),
                       img.height(),
                       3,
                       buffer,
                       3 * static_cast<int>(img.width()));

        imaging::allocator.dealloc(buffer);
    }
}

}