#include "atlas.hpp"
#include "sprite.hpp"

#include <ek/logger.hpp>
#include <platform/static_resources.hpp>

#include <ek/assets.hpp>
#include <graphics/texture.hpp>

#include <vector>
#include <ek/math/serialize_math.hpp>
#include <ek/fs/path.hpp>
#include <utils/image_loader.hpp>

using namespace ek;

namespace scenex {

/**** DTO *****/
enum class sprite_dto_flags : uint8_t {
    none = 0u,
    rotated = 1u,
    packed = 2u
};

struct sprite_dto {

    std::string name;

    // physical rect
    rect_f rc;
    // coords in atlas image
    rect_f uv;
    // flags in atlas image
    uint8_t flags = 0u;

    [[nodiscard]]
    bool is_rotated() const {
        return (flags & static_cast<uint8_t>(sprite_dto_flags::rotated)) != 0;
    }

    template<typename S>
    void serialize(IO<S>& io) {
        io(name, rc, uv, flags);
    }
};

struct atlas_page_dto {
    uint16_t width;
    uint16_t height;
    std::string image_path;
    std::vector<sprite_dto> sprites;

    template<typename S>
    void serialize(IO<S>& io) {
        io(width, height, image_path, sprites);
    }
};

struct atlas_dto {
//    std::string tag;
//    float scale;
    std::vector<atlas_page_dto> pages;

    template<typename S>
    void serialize(IO<S>& io) {
        io(pages);
    }
};

static const char* kSuffixes[] = {"", "@2x", "@3x", "@4x"};

const char* get_scale_suffix(float scale) {
    if (scale > 3.0f) {
        return kSuffixes[3];
    } else if (scale > 2.0f) {
        return kSuffixes[2];
    } else if (scale > 1.0f) {
        return kSuffixes[1];
    }
    return kSuffixes[0];
}

atlas_t::atlas_t() = default;

atlas_t::~atlas_t() {
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

void load_atlas_meta(const path_t& base_path, atlas_t* atlas, unsigned, const void* data,
                     uint32_t size) {
    // TODO: multiple pages
    input_memory_stream input{data, size};
    IO io{input};

    // header
    atlas_dto atlas_data{};
    io(atlas_data);

    for (const auto& page : atlas_data.pages) {
        auto& texture_asset = atlas->pages.emplace_back(page.image_path);
        for (auto& spr_data : page.sprites) {
            auto sprite = new sprite_t();
            sprite->rotated = spr_data.is_rotated();
            sprite->rect = spr_data.rc;
            sprite->tex = spr_data.uv;
            sprite->texture = texture_asset;

            asset_t<sprite_t> asset_spr{spr_data.name};
            asset_spr.reset(sprite);
            atlas->sprites[spr_data.name] = asset_spr;
        }
    }

    for (auto& page : atlas_data.pages) {
        const auto& page_image_path = page.image_path;
        asset_t<texture_t> texture_asset{page_image_path};
        if (texture_asset) {
            EK_DEBUG << "Destroy old page texture " << page_image_path;
            texture_asset.reset(nullptr);
        }

        EK_DEBUG << "Load atlas page " << page_image_path;
        auto* image = load_image(base_path / page.image_path);
        if (image) {
            texture_asset.reset(new texture_t);
            texture_asset->upload(*image);
            delete image;
        } else {
            EK_DEBUG << "Image not found";
        }

    }
}

atlas_t* load_atlas(const char* path, float scale_factor) {
    const path_t uid{path};
    const path_t file_meta = uid + get_scale_suffix(scale_factor) + ".atlas";
    auto* result = new atlas_t();

    auto buffer = get_resource_content(file_meta.c_str());
    if (buffer.empty()) {
        EK_DEBUG << "ATLAS META resource not found: " << file_meta;
    } else {
        load_atlas_meta(uid.dir(), result, 0u, buffer.data(), static_cast<uint32_t>(buffer.size()));
    }

    return result;
}

}