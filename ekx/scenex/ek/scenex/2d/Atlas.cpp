#include "Atlas.hpp"
#include "Sprite.hpp"

#include <ek/debug.hpp>
#include <ek/util/Res.hpp>
#include <ek/math/serialize_math.hpp>
#include <ek/serialize/stl/String.hpp>
#include <ek/util/Path.hpp>
#include <ek/app/res.hpp>
#include <ek/graphics/graphics.hpp>
#include <ek/imaging/image.hpp>

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
    Array<SpriteInfo> sprites;

    template<typename S>
    void serialize(IO<S>& io) {
        io(width, height, imagePath, sprites);
    }
};

struct AtlasInfo {
//    std::string tag;
//    float scale;
    Array<AtlasPageInfo> pages;

    template<typename S>
    void serialize(IO<S>& io) {
        io(pages);
    }
};

static const char* kSuffixes[] = {"@1x", "@2x", "@3x", "@4x"};

const char* get_scale_suffix(float scale) {
    if (scale <= 1.0f) {
        return kSuffixes[0];
    } else if (scale <= 2.0f) {
        return kSuffixes[1];
    } else if (scale <= 3.0f) {
        return kSuffixes[2];
    }
    return kSuffixes[3];
}

Atlas::Atlas() = default;

Atlas::~Atlas() {
    clear();
}

void load_atlas_meta(const path_t& base_path, Atlas* atlas, const std::vector<uint8_t>& buffer) {
    EK_DEBUG("Decoding Atlas META");
    EK_DEBUG_F("Atlas Base Path: %s", base_path.c_str());

    input_memory_stream input{buffer.data(), buffer.size()};
    IO io{input};

    // header
    AtlasInfo atlasInfo{};
    io(atlasInfo);

    for(auto* loader : atlas->loaders) {
        delete loader;
    }
    atlas->loaders.clear();

    for (const auto& page : atlasInfo.pages) {
        auto& texture_asset = atlas->pages.emplace_back(page.imagePath);
        atlas->loaders.emplace_back(new graphics::TextureLoader);
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

    for (uint32_t i = 0; i < atlasInfo.pages.size(); ++i) {
        const auto& pageInfo = atlasInfo.pages[i];
        const auto& pageImagePath = pageInfo.imagePath;
        Res<graphics::Texture> resTexture{pageImagePath};
        if (resTexture) {
            EK_DEBUG_F("Destroy old page texture %s", pageImagePath.c_str());
            resTexture.reset(nullptr);
        }

        EK_DEBUG_F("Load atlas page %s", (base_path / pageImagePath).c_str());

        auto* loader = atlas->loaders[i];
        loader->basePath = base_path.str();
        loader->imagesToLoad = 1;
        loader->urls[0] = pageImagePath;
        loader->load();
    }
}

void Atlas::load(const char* path, float scaleFactor) {
    const path_t uid{path};
    const path_t base_path = uid.dir();
    const path_t file_meta = uid + get_scale_suffix(scaleFactor) + ".atlas";

    get_resource_content_async(file_meta.c_str(), [this, file_meta, base_path](auto buffer) {
        if (buffer.empty()) {
            EK_DEBUG_F("ATLAS META resource not found: %s", file_meta.c_str());
        } else {
            load_atlas_meta(base_path, this, buffer);
        }
    });
}

int Atlas::pollLoading() {
    int toLoad = (int) loaders.size();
    if(toLoad > 0) {
        for (uint32_t i = 0; i < loaders.size(); ++i) {
            auto* loader = loaders[i];
            if (loader) {
                loader->update();
                if (!loader->loading) {
                    if (loader->status == 0) {
                        Res<graphics::Texture> res{loader->urls[0]};
                        res.reset(loader->texture);
                        delete loader;
                        loaders[i] = nullptr;
                    }
                    --toLoad;
                }
            } else {
                --toLoad;
            }
        }
        if (toLoad == 0) {
            for (auto* loader: loaders) {
                delete loader;
            }
            loaders.clear();
            return 0;
        }
        else {
            return toLoad;
        }
    }
    return 0;
}

int Atlas::getLoadingTexturesCount() const {
    int loading = 0;
    for (uint32_t i = 0; i < loaders.size(); ++i) {
        auto* loader = loaders[i];
        if (loader) {
            ++loading;
        }
    }
    return loading;
}

void Atlas::clear() {
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

}