#pragma once

#include <ek/math/box.hpp>
#include <string>
#include <utility>
#include <ek/ds/Array.hpp>
#include <ek/imaging/image.hpp>
#include <ek/math/serialize_math.hpp>

namespace pugi {
class xml_node;
}

namespace ek {

struct MultiResAtlasSettings {
    struct Resolution {
        float scale = 1.0f;
        int2 max_size{2048, 2048};
    };

    std::string name;
    Array<Resolution> resolutions{};

    void readFromXML(const pugi::xml_node& node);

    void writeToXML(pugi::xml_node& node) const;
};

enum class SpriteFlag {
    None = 0u,
    Rotated = 1u,
    Packed = 2u
};

struct SpriteData {

    std::string name;

    // physical rect
    rect_f rc;

    // coords in atlas image
    rect_f uv;

    // flags in atlas image
    uint8_t flags = 0u;

    // rect in source image
    rect_i source;

    uint8_t padding = 1;

    // reference image;
    image_t* image = nullptr;

    bool trim = false;

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

    template<typename S>
    void serialize(IO<S>& io) {
        io(name, rc, uv, flags);
    }
};

struct AtlasPageData {
    vec2_t<uint16_t> size;
    Array<SpriteData> sprites;
    std::string image_path;
    image_t* image = nullptr;

    template<typename S>
    void serialize(IO<S>& io) {
        io(size, image_path, sprites);
    }
};

struct AtlasData {
    float resolution_scale = 1.0f;
    int resolution_index = 0;
    int2 max_size{2048, 2048};
    Array<SpriteData> sprites{};
    Array<AtlasPageData> pages{};

    AtlasData() = default;

    explicit AtlasData(const MultiResAtlasSettings::Resolution& resolution,
                       int index) : resolution_scale{resolution.scale},
                                    resolution_index{index},
                                    max_size{resolution.max_size} {

    }

    template<typename S>
    void serialize(IO<S>& io) {
        io(pages);
    }
};

struct MultiResAtlasData {
    std::string name;
    Array<AtlasData> resolutions{};

    MultiResAtlasData() {
        resolutions.resize(4);
        for (size_t i = 0; i < resolutions.size(); ++i) {
            resolutions[i].resolution_index = i;
            resolutions[i].resolution_scale = float(i);
        }
    }

    explicit MultiResAtlasData(const MultiResAtlasSettings& settings) {
        int i = 0;
        resolutions.reserve(settings.resolutions.size());
        for (const auto& resolution : settings.resolutions) {
            resolutions.emplace_back(resolution, i);
            ++i;
        }
        name = settings.name;
    }

    ~MultiResAtlasData() {
        for (auto& res : resolutions) {
            for (auto& page : res.pages) {
                if (page.image) {
                    delete page.image;
                    page.image = nullptr;
                }
            }
        }
    }

    void packAndSaveMultiThreaded();
};


void saveImagePNG(const image_t& image, const std::string& path, bool alpha = true);

Array<AtlasPageData> packSprites(Array<SpriteData> sprites, int2 max_size);

}