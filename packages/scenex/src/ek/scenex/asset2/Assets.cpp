#include "Asset_impl.hpp"

#include <ek/util/Path.hpp>
#include <ek/log.h>
#include <ek/assert.h>
#include <ek/time.h>
#include <ek/audio.h>
#include <ek/local_res.h>

// texture loading
#include <ek/texture_loader.h>

#include <ek/scenex/data/ImageData.hpp>
#include <ek/gfx.h>

#include <ek/scenex/SceneFactory.hpp>
#include <ek/scenex/data/SGFile.hpp>
#include <ek/scenex/2d/Atlas.hpp>
#include <ek/scenex/3d/StaticMesh.hpp>
#include <ek/scenex/data/Model3D.hpp>

#include <ek/scenex/text/Font.hpp>
#include <ek/scenex/text/TrueTypeFont.hpp>
#include <ek/scenex/text/BitmapFont.hpp>

#include <utility>
#include <ek/scenex/2d/DynamicAtlas.hpp>
#include <ek/ds/Array.hpp>

#include "../Localization.hpp"

namespace ek {

class AudioAsset : public Asset {
public:
    AudioAsset(string_hash_t name, String filepath, uint32_t flags) :
            res{rr_named(&res_audio.rr, name)},
            path_{std::move(filepath)},
            streaming{flags != 0} {
    }

    void do_load() override {
        fullPath_ = manager_->base_path / path_;
        auph_buffer* buffer = &REF_RESOLVE(res_audio, res);
        // if assertion is triggering - implement cleaning up the slot before loading
        EK_ASSERT(buffer->id == 0);
        *buffer = auph_load(fullPath_.c_str(), streaming ? AUPH_FLAG_STREAM : 0);
    }

    void poll() override {
        auto buffer = REF_RESOLVE(res_audio, res);
        auto failed = !auph_is_active(buffer.id);
        auto completed = auph_is_buffer_loaded(buffer);
        if (failed || completed) {
            state = AssetState::Ready;
        }
    }

    void do_unload() override {
        if (res) {
            auph_buffer* audio = &REF_RESOLVE(res_audio, res);
            if (audio->id && auph_is_active(audio->id)) {
                auph_unload(*audio);
            }
            audio->id = 0;
        }
    }

    REF_TO(auph_buffer) res;
    String path_;
    String fullPath_;
    bool streaming = false;
};

class AtlasAsset : public Asset {
public:
    AtlasAsset(const char* name_, uint32_t formatMask_) : res{H(name_)},
                                                          name{name_} {
        // we need to load atlas image and atlas meta
        weight_ = 2;
        formatMask = formatMask_;
    }

    void load() override {
        if (state != AssetState::Ready || loaded_scale_ != manager_->scale_uid) {
            loaded_scale_ = manager_->scale_uid;

            res.reset(nullptr);

            fullPath_ = manager_->base_path / name;

            if (!res) {
                res.reset(new Atlas);
                // do not switch to loading state, because after first load system does not poll pack's Asset objects
                state = AssetState::Loading;
            }

            res->formatMask = formatMask;
            res->load(fullPath_.c_str(), manager_->scale_factor);
        }
    }

    void poll() override {
        if (state == AssetState::Loading) {
            if (!res.empty()) {
                // we poll atlas loading / reloading in separated process with Atlas::pollLoading for each Res<Atlas>
                int loading = res->getLoadingImagesCount();
                if (loading == 0) {
                    state = AssetState::Ready;
                }
            }
        }
    }

    [[nodiscard]]
    float getProgress() const override {
        if (state == AssetState::Loading) {
            float progress = 0.0f;
            if (!res.empty()) {
                progress = 1.0f;
                const auto totalPages = (float) res->pages.size();
                float loadedPages = 0.0f;
                for (auto page: res->pages) {
                    if (REF_RESOLVE(res_image, page).id) {
                        loadedPages += 1.0f;
                    }
                }
                if (totalPages > 0.0f) {
                    progress += loadedPages / totalPages;
                } else {
                    progress = 2.0f;
                }
            }
            return progress;
        }
        return Asset::getProgress();
    }

    void do_unload() override {
        res.reset(nullptr);
    }

    Res<Atlas> res;
    String name;
    uint8_t loaded_scale_ = 0;
    String fullPath_;
    uint32_t formatMask = 1;
};

class DynamicAtlasAsset : public Asset {
public:
    DynamicAtlasAsset(string_hash_t name, uint32_t flags) : res{name}, flags_{flags} {
    }

    // do not reload dynamic atlas, because references to texture* should be invalidated,
    // but current strategy not allow that
    void do_load() override {
        const int pageSize = DynamicAtlas::estimateBetterSize(manager_->scale_factor,
                                                              512,
                                                              2048);
        res.reset(
                new DynamicAtlas(
                        pageSize,
                        pageSize,
                        (flags_ & 1) != 0,
                        (flags_ & 2) != 0
                )
        );
        state = AssetState::Ready;
    }

    void do_unload() override {
        res.reset(nullptr);
    }

    Res<DynamicAtlas> res;
    uint32_t flags_ = 0;
};

class SceneAsset : public Asset {
public:
    explicit SceneAsset(string_hash_t name, String path) :
            res{name},
            path_{std::move(path)} {
    }

    void do_load() override {
        fullPath_ = manager_->base_path / path_;
        ek_local_res_load(fullPath_.c_str(),
                          [](ek_local_res* lr) {
                              SceneAsset* this_ = (SceneAsset*) lr->userdata;
                              if (ek_local_res_success(lr)) {
                                  this_->res.reset(sg_load(lr->buffer, (uint32_t) lr->length));
                              }
                              ek_local_res_close(lr);
                              this_->state = AssetState::Ready;
                          }, this);
    }

    void do_unload() override {
        res.reset(nullptr);
    }

    Res<SGFile> res;
    String path_;
    String fullPath_;
};

class BitmapFontAsset : public Asset {
public:
    BitmapFontAsset(string_hash_t name, String path) :
            res{name},
            path_{std::move(path)} {
    }

    void do_load() override {
        fullPath_ = manager_->base_path / path_;
        ek_local_res_load(
                fullPath_.c_str(),
                [](ek_local_res* lr) {
                    BitmapFontAsset* this_ = (BitmapFontAsset*) lr->userdata;
                    if (ek_local_res_success(lr)) {
                        auto* bmFont = new BitmapFont();
                        bmFont->load(lr->buffer, lr->length);
                        this_->res.reset(new Font(bmFont));
                    }
                    ek_local_res_close(lr);
                    this_->state = AssetState::Ready;
                }, this);
    }

    void do_unload() override {
        res.reset(nullptr);
    }

    Res<Font> res;
    String path_;
    String fullPath_;
};

class ImageAsset : public Asset {
public:
    ImageAsset(string_hash_t name, ImageData data) :
            res{rr_named(&res_image.rr, name)},
            data_{std::move(data)} {
        weight_ = (float) data_.images.size();
    }

    void do_load() override {
        loader = ek_texture_loader_create();
        ek_texture_loader_set_path(&loader->basePath, manager_->base_path.c_str());
        EK_ASSERT(data_.images.size() <= EK_TEXTURE_LOADER_IMAGES_MAX_COUNT);
        for (int i = 0; i < data_.images.size(); ++i) {
            ek_texture_loader_set_path(loader->urls + i, data_.images[i].c_str());
        }
        loader->imagesToLoad = (int) data_.images.size();
        if (data_.type == ImageDataType::CubeMap) {
            loader->isCubeMap = true;
            loader->premultiplyAlpha = false;
            loader->formatMask = data_.formatMask;
        }
        ek_texture_loader_load(loader);
        state = AssetState::Loading;
    }

    void poll() override {
        if (loader) {
            if (loader->loading) {
                ek_texture_loader_update(loader);
            }

            if (!loader->loading) {
                error = loader->status;
                if (error == 0) {
                    REF_RESOLVE(res_image, res) = loader->image;
                }
                state = AssetState::Ready;
                ek_texture_loader_destroy(loader);
                loader = nullptr;
            }
        }
    }

    [[nodiscard]]
    float getProgress() const override {
        if (state == AssetState::Loading) {
            return loader ? loader->progress : 0.0f;
        }
        return Asset::getProgress();
    }

    void do_unload() override {
        if (res) {
            sg_image* image = &REF_RESOLVE(res_image, res);
            if (image->id) {
                sg_destroy_image(*image);
                *image = {SG_INVALID_ID};
            }
        }
    }

    REF_TO(sg_image) res;
    ek_texture_loader* loader = nullptr;
    ImageData data_{};
    // by default always premultiply alpha,
    // currently for cube maps will be disabled
    // TODO: export level option
    bool premultiplyAlpha = true;
};

class StringsAsset : public Asset {
public:
    StringsAsset(String name, Array<String> langs) :
            name_{std::move(name)} {
        for (auto& l: langs) {
            loaders_.push_back({this, std::move(l)});
        }
        weight_ = (float) loaders_.size();
    }

    void do_load() override {
        loaded = 0;
        for (int i = 0; i < loaders_.size(); ++i) {
            auto* loader = &loaders_[i];
            auto langPath = (manager_->base_path / name_ / loader->lang) + ".mo";
            ek_local_res_load(
                    langPath.c_str(),
                    [](ek_local_res* lr) {
                        lang_loader* loader_ = (lang_loader*) lr->userdata;
                        const char* lang = loader_->lang.c_str();
                        StringsAsset* asset = loader_->asset;
                        if (ek_local_res_success(lr)) {
                            Localization::instance.load(lang, *lr);
                        } else {
                            EK_ERROR("Strings resource not found: %s", lang);
                            asset->error = 1;
                        }
                        ++asset->loaded;
                        if (asset->loaded >= asset->loaders_.size()) {
                            asset->state = AssetState::Ready;
                        }
                    },
                    loader
            );
        }
    }

    void do_unload() override {

    }

    struct lang_loader {
        StringsAsset* asset;
        String lang;
    };

    String name_;
    Array<lang_loader> loaders_;
    uint32_t loaded = 0;

};

class ModelAsset : public Asset {
public:
    explicit ModelAsset(String name) :
            name_{std::move(name)} {
    }

    void do_load() override {
        fullPath_ = manager_->base_path / name_ + ".model";
        ek_local_res_load(
                fullPath_.c_str(),
                [](ek_local_res* lr) {
                    ModelAsset* this_ = (ModelAsset*) lr->userdata;
                    if (ek_local_res_success(lr)) {
                        input_memory_stream input{lr->buffer, lr->length};
                        IO io{input};
                        Model3D model;
                        io(model);
                        Res<StaticMesh>{H(this_->name_.c_str())}.reset(new StaticMesh(model));
                        ek_local_res_close(lr);
                    } else {
                        EK_ERROR("MODEL resource not found: %s", this_->fullPath_.c_str());
                        this_->error = 1;
                    }
                    this_->state = AssetState::Ready;
                },
                this
        );
    }

    void do_unload() override {
        Res<StaticMesh>{H(name_.c_str())}.reset(nullptr);
    }

    String name_;
    String fullPath_;
};

bool isTimeBudgetAllowStartNextJob(uint64_t since) {
    return ek_ticks_to_sec(ek_ticks(&since)) < 0.008;
}

class PackAsset : public Asset {
public:

    bool assetListLoaded = false;

    explicit PackAsset(String name) :
            name_{std::move(name)} {
    }

    void do_load() override {
        assetListLoaded = false;
        assetsLoaded = 0;
        fullPath_ = manager_->base_path / name_;
        ek_local_res_load(
                fullPath_.c_str(),
                [](ek_local_res* lr) {
                    PackAsset* this_ = (PackAsset*) lr->userdata;
                    if (ek_local_res_success(lr)) {
                        input_memory_stream input{lr->buffer, lr->length};
                        IO io{input};
                        bool end = false;
                        while (!end) {
                            uint32_t headerSize = 0;
                            io(headerSize);
                            if (headerSize != 0) {
                                auto* asset = this_->manager_->add_from_type(io.stream.dataAtPosition(), headerSize);
                                if (asset) {
                                    this_->assets.push_back(asset);
                                }
                                io.stream.seek((int32_t) headerSize);
                            } else {
                                end = true;
                            }
                        }
                        ek_local_res_close(lr);
                    }
                    // ready for loading
                    this_->assetListLoaded = true;
                },
                this
        );
    }

    void do_unload() override {
        for (auto asset: assets) {
            asset->unload();
        }
        assets.clear();
        assetsLoaded = 0;
        assetListLoaded = false;
    }

    void poll() override {
        if (state != AssetState::Loading || !assetListLoaded) {
            return;
        }

        uint64_t timer = ek_ticks(nullptr);

        unsigned numAssetsLoaded = 0;
        for (auto asset: assets) {
            const auto initialState = asset->state;
            if (asset->state == AssetState::Initial) {
                if (isTimeBudgetAllowStartNextJob(timer)) {
//                    EK_DEBUG("Loading BEGIN: %s", asset->name_.c_str());
                    asset->load();
                }
            }
            if (asset->state == AssetState::Loading) {
                if (isTimeBudgetAllowStartNextJob(timer)) {
                    asset->poll();
                }
            }
            if (asset->state == AssetState::Ready) {
                if (initialState != AssetState::Ready) {
//                    EK_DEBUG("Loading END: %s", asset->name_.c_str());
                }
                ++numAssetsLoaded;
            }
        }

        if (!isTimeBudgetAllowStartNextJob(timer)) {
            uint64_t since = timer;
            double elapsed = ek_ticks_to_sec(ek_ticks(&since));
            EK_INFO("Assets loading jobs spend %d ms", (int) (elapsed * 1000));
        }

        assetsLoaded = numAssetsLoaded;
        if (numAssetsLoaded >= assets.size()) {
            state = AssetState::Ready;
        }
    }

    [[nodiscard]] float getProgress() const override {
        switch (state) {
            case AssetState::Ready:
                return 1.0f;
            case AssetState::Initial:
                return 0.0f;
            case AssetState::Loading:
                // calculate sub-assets progress
                if (!assets.empty()) {
                    float acc = 0.0f;
                    float total = 0.0f;
                    for (auto asset: assets) {
                        const float w = asset->weight_;
                        acc += w * asset->getProgress();
                        total += w;
                    }
                    if (total > 0.0f) {
                        return acc / total;
                    }
                }
        }
        return 0.0f;
    }

    String name_;
    String fullPath_;
    Array<Asset*> assets;
    unsigned assetsLoaded = 0;
};

class TrueTypeFontAsset : public Asset {
public:

    TrueTypeFontAsset(string_hash_t name, String path, string_hash_t glyphCache, float baseFontSize) :
            res{name},
            baseFontSize_{baseFontSize},
            path_{std::move(path)},
            glyphCache_{glyphCache} {
    }

    void do_load() override {
        fullPath_ = manager_->base_path / path_;
        ek_local_res_load(
                fullPath_.c_str(),
                [](ek_local_res* lr) {
                    TrueTypeFontAsset* this_ = (TrueTypeFontAsset*) lr->userdata;
                    TrueTypeFont* ttfFont = new TrueTypeFont(this_->manager_->scale_factor, this_->baseFontSize_,
                                                             this_->glyphCache_);
                    ttfFont->loadFromMemory(lr);
                    this_->res.reset(new Font(ttfFont));
                    this_->state = AssetState::Ready;
                }, this);
    }

    void do_unload() override {
        res.reset(nullptr);
    }

    Res<Font> res;
    float baseFontSize_;
    String path_;
    String fullPath_;
    string_hash_t glyphCache_;
};

Asset* DefaultAssetsResolver::create_from_file(const String& path, const String& type) const {
    if (type == "pack") {
        return new PackAsset(path);
    }
    return nullptr;
}

Asset* DefaultAssetsResolver::create(const String& path) const {
    (void) path;
    return nullptr;
}

bool io_string_view_equals(IOStringView view, const char* lit) {
    return strncmp(view.data, lit, view.size) == 0;
}

Asset* DefaultAssetsResolver::create_for_type(const void* data, uint32_t size) const {
    input_memory_stream stream{data, size};
    IO io{stream};
    string_hash_t type;
    io(type);
    if (type == H("audio")) {
        string_hash_t name;
        uint32_t flags = 0;
        String path;
        io(name, flags, path);
        return new AudioAsset(name, path, flags);
    } else if (type == H("scene")) {
        string_hash_t name;
        String path;
        io(name, path);
        return new SceneAsset(name, path);
    } else if (type == H("bmfont")) {
        string_hash_t name;
        String path;
        io(name, path);
        return new BitmapFontAsset(name, path);
    } else if (type == H("ttf")) {
        string_hash_t name;
        String path;
        string_hash_t glyphCache;
        float baseFontSize;
        io(name, path, glyphCache, baseFontSize);
        return new TrueTypeFontAsset(name, path, glyphCache, baseFontSize);
    } else if (type == H("atlas")) {
        IOStringView name;
        uint32_t formatMask = 1;
        io(name, formatMask);
        return new AtlasAsset(name.data, formatMask);
    } else if (type == H("dynamic_atlas")) {
        string_hash_t name;
        uint32_t flags;
        io(name, flags);
        return new DynamicAtlasAsset(name, flags);
    } else if (type == H("model")) {
        String name;
        io(name);
        return new ModelAsset(name);
    } else if (type == H("texture")) {
        string_hash_t name;
        ImageData texData;
        io(name, texData);
        return new ImageAsset(name, texData);
    } else if (type == H("strings")) {
        String name;
        Array<String> langs;
        io(name, langs);
        return new StringsAsset(name, langs);
    } else if (type == H("pack")) {
        String name;
        io(name);
        return new PackAsset(name);
    }
    return nullptr;
}

}