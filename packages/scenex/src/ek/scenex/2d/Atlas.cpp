#include "Atlas.hpp"
#include "Sprite.hpp"

#include <ek/log.h>
#include <ek/util/Res.hpp>
#include <ek/math/MathSerialize.hpp>
#include <ek/serialize/serialize.hpp>
#include <ek/util/Path.hpp>
#include <ek/local_res.hpp>
#include <ek/graphics/graphics.hpp>
#include <ek/image.h>
#include <stb/stb_sprintf.h>

namespace ek {

/**** DTO *****/
struct SpriteInfo {
    enum Flags : uint8_t {
        None = 0u,
        Rotated = 1u
    };

    String name;

    // physical rect
    Rect2f rc;

    // coords in atlas image
    Rect2f uv;

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
    String imagePath;
    Array<SpriteInfo> sprites;

    template<typename S>
    void serialize(IO<S>& io) {
        io(width, height, imagePath, sprites);
    }
};

struct AtlasInfo {
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

void load_atlas_meta(const char* base_path, Atlas* atlas, ek_local_res lr) {
    EK_DEBUG("Decoding Atlas META");
    EK_DEBUG("Atlas Base Path: %s", base_path);

    input_memory_stream input{lr.buffer, lr.length};
    IO io{input};

    // header
    AtlasInfo atlasInfo{};
    io(atlasInfo);

    for (auto* loader: atlas->loaders) {
        ek_texture_loader_destroy(loader);
    }
    atlas->loaders.clear();

    for (const auto& page: atlasInfo.pages) {
        auto& texture_asset = atlas->pages.emplace_back(page.imagePath.c_str());
        atlas->loaders.emplace_back(ek_texture_loader_create());
        for (auto& spr_data: page.sprites) {
            auto sprite = new Sprite();
            sprite->rotated = spr_data.isRotated();
            sprite->rect = spr_data.rc;
            sprite->tex = spr_data.uv;
            sprite->texture = texture_asset;

            Res<Sprite> asset_spr{spr_data.name.c_str()};
            asset_spr.reset(sprite);
            atlas->sprites.push_back(asset_spr);
        }
    }

    for (uint32_t i = 0; i < atlasInfo.pages.size(); ++i) {
        const auto& pageInfo = atlasInfo.pages[i];
        const auto& pageImagePath = pageInfo.imagePath;
        Res<graphics::Texture> resTexture{pageImagePath.c_str()};
        if (resTexture) {
            EK_DEBUG("Destroy old page texture %s", pageImagePath.c_str());
            resTexture.reset(nullptr);
        }

        EK_DEBUG("Load atlas page %s/%s", base_path, pageImagePath.c_str());

        auto* loader = atlas->loaders[i];
        loader->formatMask = atlas->formatMask;
        ek_texture_loader_set_path(&loader->basePath, base_path);
        loader->imagesToLoad = 1;
        ek_texture_loader_set_path(&loader->urls[0], pageImagePath.c_str());
        ek_texture_loader_load(loader);
    }
}

void Atlas::load(const char* path, float scaleFactor) {
    char tmp[1024];
    ek_snprintf(tmp, 1024, "%s%s.atlas", path, get_scale_suffix(scaleFactor));
    const String metaFilePath{tmp};
    const String basePath = Path::directory(path);
    get_resource_content_async(
            metaFilePath.c_str(),
            [this, metaFilePath, basePath](ek_local_res lr) {
                if (ek_local_res_success(&lr)) {
                    load_atlas_meta(basePath.c_str(), this, lr);
                } else {
                    EK_DEBUG("ATLAS META resource not found: %s", metaFilePath.c_str());
                }
                ek_local_res_close(&lr);
            }
    );
}

int Atlas::pollLoading() {
    int toLoad = (int) loaders.size();
    if (toLoad > 0) {
        for (uint32_t i = 0; i < loaders.size(); ++i) {
            auto* loader = loaders[i];
            if (loader) {
                ek_texture_loader_update(loader);
                if (!loader->loading) {
                    if (loader->status == 0) {
                        Res<graphics::Texture> res{loader->urls[0].path};
                        res.reset(new graphics::Texture{loader->image});
                        ek_texture_loader_destroy(loader);
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
                if(loader) {
                    ek_texture_loader_destroy(loader);
                }
            }
            loaders.clear();
            return 0;
        } else {
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

    for (auto& page: pages) {
        page.reset(nullptr);
    }

    for (auto& spr: sprites) {
        spr.reset(nullptr);
    }
}

}