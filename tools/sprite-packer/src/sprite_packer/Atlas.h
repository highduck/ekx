#pragma once

#include "common.h"
#include "sprpk_image.h"

namespace sprite_packer {

class Bitmap;

struct ResolutionConfig {
    float scale = 1.0f;
    int maxWidth = 2048;
    int maxHeight = 2048;
};

struct AtlasConfig {


    std::string name;
    std::vector<ResolutionConfig> resolutions;

    void readFromXML(const pugi::xml_node& node) {
        name = node.attribute("name").as_string();
        for (auto& resolution_node: node.children("resolution")) {
            ResolutionConfig res{};
            res.scale = resolution_node.attribute("scale").as_float(res.scale);
            res.maxWidth = resolution_node.attribute("max_width").as_int(res.maxWidth);
            res.maxHeight = resolution_node.attribute("max_height").as_int(res.maxHeight);
            resolutions.push_back(res);
        }
    }
};

typedef enum sprite_flag_t {
    SPRITE_FLAG_ROTATED = 1u,
    SPRITE_FLAG_PACKED = 2u,
    SPRITE_FLAG_TRIM = 4u
} sprite_flag_t;

struct SpriteData {

    std::string name;

    // physical rect
    rect_t rc;

    // coords in atlas image
    rect_t uv;

    // flags in atlas image
    uint8_t flags = 0u;

    // rect in source image
    irect_t source;

    uint8_t padding = 1;

    // reference image;
    ek_bitmap bitmap = {0, 0, nullptr};
};

struct PageData {
    uint16_t w;
    uint16_t h;
    std::vector<SpriteData> sprites;
    std::string image_path;
    ek_bitmap bitmap;

//    template<typename S>
//    void serialize(IO <S>& io) {
//        io(size, image_path, sprites);
//    }
};

struct AtlasData {
    float resolution_scale = 1.0f;
    int resolution_index = 0;
    int maxWidth = 2048;
    int maxHeight = 2048;
    std::vector<SpriteData> sprites;
    std::vector<PageData> pages;

    AtlasData() = default;

    AtlasData(const ResolutionConfig& resolution, int index) : resolution_scale{resolution.scale},
                                                               resolution_index{index},
                                                               maxWidth{resolution.maxWidth},
                                                               maxHeight{resolution.maxHeight} {

    }

//    template<typename S>
//    void serialize(IO <S>& io) {
//        io(pages);
//    }
};

struct Atlas {
    std::string name;
    std::vector<AtlasData> resolutions;

    Atlas() {
        resolutions.resize(4);
        for (size_t i = 0; i < resolutions.size(); ++i) {
            resolutions[i].resolution_index = i;
            resolutions[i].resolution_scale = float(i);
        }
    }

    explicit Atlas(const AtlasConfig& config) {
        int i = 0;
        resolutions.reserve(config.resolutions.size());
        for (const auto& resolution: config.resolutions) {
            resolutions.emplace_back(resolution, i);
            ++i;
        }
        name = config.name;
    }

    ~Atlas() {
        for (auto& res: resolutions) {
            for (auto& page: res.pages) {
                ek_bitmap_free(&page.bitmap);
            }
        }
    }

    void packAndSaveMultiThreaded(const char* outputPath);
};

}
