#include "Atlas.hpp"
#include "Sprite.hpp"

#include <ek/log.h>

#include <ek/serialize/serialize.hpp>
#include <ek/ds/String.hpp>
#include <ek/ds/PodArray.hpp>
#include <ek/local_res.h>
#include <ek/gfx.h>
#include <ek/print.h>

struct res_atlas res_atlas;

void setup_res_atlas(void) {
    struct res_atlas* R = &res_atlas;
    rr_man_t* rr = &R->rr;

    rr->names = R->names;
    rr->data = R->data;
    rr->max = sizeof(R->data) / sizeof(R->data[0]);
    rr->num = 1;
    rr->data_size = sizeof(R->data[0]);
}

void update_res_atlas(void) {
    for (res_id id = 0; id < res_atlas.rr.num; ++id) {
        atlas_ptr content = res_atlas.data[id];
        if (content) {
            content->pollLoading();
        }
    }
}


namespace ek {

/**** DTO *****/
struct SpriteInfo {
    enum Flags {
        Rotated = 1u
    };

    string_hash_t name;

    // flags in atlas image
    uint32_t flags;

    // physical rect
    rect_t rc;

    // coords in atlas image
    rect_t uv;

    [[nodiscard]]
    inline bool isRotated() const {
        return (flags & Rotated) != 0;
    }
};

template<> struct declared_as_pod_type<SpriteInfo> : public std::true_type {};

struct AtlasPageInfo {
    uint16_t width;
    uint16_t height;
    String imagePath;
    PodArray<SpriteInfo> sprites;

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

int get_scale_num(float scale) {
    if (scale <= 1.0f) {
        return 1;
    } else if (scale <= 2.0f) {
        return 2;
    } else if (scale <= 3.0f) {
        return 3;
    }
    return 4;
}

Atlas::Atlas() = default;

Atlas::~Atlas() {
    clear();
}

void load_atlas_meta(Atlas* atlas, ek_local_res* lr) {
    log_debug("Decoding Atlas META");
    log_debug("Atlas Base Path: %s", atlas->base_path.c_str());

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
        auto image_asset = R_IMAGE(H(page.imagePath.c_str()));

        sg_image* image = &REF_RESOLVE(res_image, image_asset);
        if (image->id) {
            log_debug("Destroy old page image %s", page.imagePath.c_str());
            sg_destroy_image(*image);
            *image = {SG_INVALID_ID};
        }

        atlas->pages.push_back(image_asset);
        atlas->loaders.emplace_back(ek_texture_loader_create());
        for (auto& spr_data: page.sprites) {
            auto ref = R_SPRITE(spr_data.name);
            atlas->sprites.push_back(ref);

            auto* sprite = &REF_RESOLVE(res_sprite, ref);
            EK_ASSERT(!(sprite->state & SPRITE_LOADED));
            sprite->state = SPRITE_LOADED;
            if(spr_data.flags & SpriteInfo::Rotated) {
                sprite->state |= SPRITE_ROTATED;
            }
            sprite->image_id = image_asset;
            sprite->rect = spr_data.rc;
            sprite->tex = spr_data.uv;
        }
    }

    for (uint32_t i = 0; i < atlasInfo.pages.size(); ++i) {
        const auto& pageInfo = atlasInfo.pages[i];
        const auto& pageImagePath = pageInfo.imagePath;

        log_debug("Load atlas page %s/%s", atlas->base_path.c_str(), pageImagePath.c_str());

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
    ek_snprintf(meta_file_path, sizeof meta_file_path, "%s%dx.atlas", path, get_scale_num(scaleFactor));

    char dir_buf[1024];
    ek_path_dirname(dir_buf, sizeof dir_buf, path);
    base_path = dir_buf;

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
                        // ref = ek_ref_make(sg_image, loader->urls[0].path)
                        // ek_ref_clear(ref)
                        // item = ek_ref_get_item(ref)
                        // item->handle = loader->image;
                        // item->finalizer = sg_image_REF_finalizer
                        const res_id image_id = R_IMAGE(H(loader->urls[0].path));
                        REF_RESOLVE(res_image, image_id) = loader->image;
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

int Atlas::getLoadingImagesCount() const {
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

    for (auto page: pages) {
        sg_image* image = &REF_RESOLVE(res_image, page);
        if (image->id) {
            sg_destroy_image(*image);
            *image = {SG_INVALID_ID};
        }
    }

    for (auto ref: sprites) {
        auto* spr = &REF_RESOLVE(res_sprite, ref);
        spr->image_id = 0;
        spr->state = 0;
    }
}

}