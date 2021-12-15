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


enum class SpriteFlag {
    None = 0u,
    Rotated = 1u,
    Packed = 2u,
    Trim = 4u
};

struct SpriteData {

    std::string name;

    // physical rect
    ek_img_rect_f rc;

    // coords in atlas image
    ek_img_rect_f uv;

    // flags in atlas image
    uint8_t flags = 0u;

    // rect in source image
    ek_img_rect source;

    uint8_t padding = 1;

    // reference image;
    ek_image bitmap = {0, 0, nullptr};

    [[nodiscard]]
    bool is_packed() const {
        return (flags & static_cast<uint8_t>(SpriteFlag::Packed)) != 0;
    }

    [[nodiscard]]
    bool is_rotated() const {
        return (flags & static_cast<uint8_t>(SpriteFlag::Rotated)) != 0;
    }

    void enable(SpriteFlag flag) {
        flags |= static_cast<uint8_t>(flag);
    }

    void disable(SpriteFlag flag) {
        flags &= ~static_cast<uint8_t>(flag);
    }

//    template<typename S>
//    void serialize(IO <S>& io) {
//        io(name, rc, uv, flags);
//    }
};

struct PageData {
    uint16_t w;
    uint16_t h;
    std::vector<SpriteData> sprites;
    std::string image_path;
    ek_image bitmap;

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
