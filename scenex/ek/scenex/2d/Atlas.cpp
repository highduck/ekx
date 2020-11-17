#include "Atlas.hpp"
#include "Sprite.hpp"

#include <ek/util/logger.hpp>
#include <ek/util/assets.hpp>
#include <ek/math/serialize_math.hpp>
#include <ek/util/path.hpp>
#include <ek/app/res.hpp>
#include <ek/graphics/texture.hpp>

#include <vector>

namespace ek {

/**** DTO *****/
struct SpriteInfo {
    enum Flags : uint8_t {
        None = 0u,
        Rotated = 1u
    };

    std::string name;

    // physical rect
    rect_f rc;

    // coords in atlas image
    rect_f uv;

    // flags in atlas image
    uint8_t flags = 0u;

    [[nodiscard]]
    inline bool isRotated() const {
        return (flags & Rotated) != 0;
    }

    template<typename S>
    void serialize(IO<S>& io) {
        io(name, rc, uv, flags);
    }
};

struct AtlasPageInfo {
    uint16_t width;
    uint16_t height;
    std::string imagePath;
    std::vector<SpriteInfo> sprites;

    template<typename S>
    void serialize(IO<S>& io) {
        io(width, height, imagePath, sprites);
    }
};

struct AtlasInfo {
//    std::string tag;
//    float scale;
    std::vector<AtlasPageInfo> pages;

    template<typename S>
    void serialize(IO<S>& io) {
        io(pages);
    }
};

static const char *kSuffixes[] = {"", "@2x", "@3x", "@4x"};

const char *get_scale_suffix(float scale) {
    if (scale > 3.0f) {
        return kSuffixes[3];
    } else if (scale > 2.0f) {
        return kSuffixes[2];
    } else if (scale > 1.0f) {
        return kSuffixes[1];
    }
    return kSuffixes[0];
}

Atlas::Atlas() = default;

Atlas::~Atlas() {
    // TODO: idea with ref counting and when we can unload - just delete all unreferenced resources
//    for (const auto& texture : mPrivate->textures) {
//        asset_t<texture_t>::unload(texture);
//    }

    for (auto& page : pages) {
        page.reset(nullptr);
    }

    for (auto& pair : sprites) {
        pair.second.reset(nullptr);
    }
}

void load_atlas_meta(const path_t& base_path, Atlas *atlas, const std::vector<uint8_t>& buffer) {
    EK_DEBUG << "Decoding Atlas META";
    EK_DEBUG << "Atlas Base Path: " << base_path;

    input_memory_stream input{buffer.data(), buffer.size()};
    IO io{input};

    // header
    AtlasInfo atlasInfo{};
    io(atlasInfo);

    for (const auto& page : atlasInfo.pages) {
        auto& texture_asset = atlas->pages.emplace_back(page.imagePath);
        for (auto& spr_data : page.sprites) {
            auto sprite = new Sprite();
            sprite->rotated = spr_data.isRotated();
            sprite->rect = spr_data.rc;
            sprite->tex = spr_data.uv;
            sprite->texture = texture_asset;

            Res<Sprite> asset_spr{spr_data.name};
            asset_spr.reset(sprite);
            atlas->sprites[spr_data.name] = asset_spr;
        }
    }

    for (auto& page : atlasInfo.pages) {
        const auto& page_image_path = page.imagePath;
        Res<graphics::texture_t> resTexture{page_image_path};
        if (resTexture) {
            EK_DEBUG << "Destroy old page texture " << page_image_path;
            resTexture.reset(nullptr);
        }

        EK_DEBUG << "Load atlas page " << (base_path / page_image_path);
//        get_resource_content_async((base_path / page_image_path).c_str(), [page_image_path](auto image_buffer) {
//            if (image_buffer.empty()) {
//                EK_DEBUG << "Image not found";
//            } else {
//                auto* image = decode_image_data(image_buffer);
//                if (image) {
//                    asset_t<texture_t> texture_asset{page_image_path};
//                    texture_asset.reset(new texture_t);
//                    texture_asset->upload(*image);
//                    delete image;
//                }
//            }
//        });
        resTexture.reset(new graphics::texture_t);
        resTexture->reset(1, 1);
        load_texture_lazy((base_path / page_image_path).c_str(), resTexture.get_mutable());
    }
}

void Atlas::load(const char *path, float scale_factor, const Atlas::LoadCallback& callback) {
    const path_t uid{path};
    const path_t base_path = uid.dir();
    const path_t file_meta = uid + get_scale_suffix(scale_factor) + ".atlas";

    get_resource_content_async(file_meta.c_str(), [callback, file_meta, base_path](auto buffer) {
        Atlas *atlas = nullptr;
        if (buffer.empty()) {
            EK_DEBUG("ATLAS META resource not found: %s", file_meta.c_str());
        } else {
            atlas = new Atlas;
            load_atlas_meta(base_path, atlas, buffer);
        }
        callback(atlas);
    });
}

}