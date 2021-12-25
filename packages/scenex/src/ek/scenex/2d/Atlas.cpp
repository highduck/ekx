#include "Atlas.hpp"
#include "Sprite.hpp"

#include <ek/log.h>
#include <ek/util/Res.hpp>
#include <ek/math/MathSerialize.hpp>
#include <ek/serialize/serialize.hpp>
#include <ek/util/Path.hpp>
#include <ek/local_res.h>
#include <ek/graphics/graphics.hpp>
#include <ek/image.h>
#include <ek/print.h>

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

void load_atlas_meta(Atlas* atlas, ek_local_res* lr) {
    EK_DEBUG("Decoding Atlas META");
    EK_DEBUG("Atlas Base Path: %s", atlas->base_path.c_str());

    input_memory_stream input{lr->buffer, lr->length};
    IO io{input};

    // header
    AtlasInfo atlasInfo{};
    io(atlasInfo);

    for (auto* loader: atlas->loaders) {
        ek_texture_loader_destroy(loader);
    }
    atlas->loaders.clear();

    for (const auto& page: atlasInfo.pages) {
        auto texture_asset = ek_texture_reg_named(page.imagePath.c_str());
        atlas->pages.push_back(texture_asset);
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
        const ek_texture_reg_id texture_id = ek_texture_reg_named(pageImagePath.c_str());
        const sg_image image = ek_texture_reg_get(texture_id);
        if (image.id) {
            EK_DEBUG("Destroy old page texture %s", pageImagePath.c_str());
            ek_texture_reg_assign(texture_id, {SG_INVALID_ID});
        }

        EK_DEBUG("Load atlas page %s/%s", atlas->base_path.c_str(), pageImagePath.c_str());

        auto* loader = atlas->loaders[i];
        loader->formatMask = atlas->formatMask;
        ek_texture_loader_set_path(&loader->basePath, atlas->base_path.c_str());
        loader->imagesToLoad = 1;
        ek_texture_loader_set_path(&loader->urls[0], pageImagePath.c_str());
        ek_texture_loader_load(loader);
    }
}

void Atlas::load(const char* path, float scaleFactor) {
    char meta_file_path[1024];
    ek_snprintf(meta_file_path, sizeof meta_file_path, "%s%s.atlas", path, get_scale_suffix(scaleFactor));
    base_path = Path::directory(path);
    ek_local_res_load(
            meta_file_path,
            [](ek_local_res* lr) {
                Atlas* this_ = (Atlas*) lr->userdata;
                if (ek_local_res_success(lr)) {
                    load_atlas_meta(this_, lr);
                }
                ek_local_res_close(lr);
            },
            this
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
                        ek_texture_reg_id res = ek_texture_reg_named(loader->urls[0].path);
                        ek_texture_reg_assign(res, loader->image);
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
                if (loader) {
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

    for (auto page: pages) {
        ek_texture_reg_assign(page, {SG_INVALID_ID});
    }

    for (auto& spr: sprites) {
        spr.reset(nullptr);
    }
}

}