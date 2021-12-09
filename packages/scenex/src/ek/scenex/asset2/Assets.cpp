#include "Asset_impl.hpp"

#include <ek/util/Path.hpp>
#include <ek/log.h>
#include <ek/assert.h>
#include <ek/time.h>
#include <ek/audio/audio.hpp>
#include <ek/LocalResource.hpp>

// texture loading
#include <ek/TextureLoader.hpp>

#include <ek/scenex/data/TextureData.hpp>
#include <ek/graphics/graphics.hpp>

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

using graphics::Texture;
using graphics::Shader;

class AudioAsset : public Asset {
public:
    AudioAsset(const char* name, String filepath, uint32_t flags) :
            res{name},
            path_{std::move(filepath)},
            streaming{flags != 0} {
    }

    void do_load() override {
        auto* audio = new audio::AudioResource();

        fullPath_ = manager_->base_path / path_;
        audio->load(fullPath_.c_str(), streaming);
        res.reset(audio);
    }

    void poll() override {
        auto buffer = res->buffer;
        auto failed = !auph::isActive(buffer.id);
        auto completed = auph::isBufferLoaded(buffer);
        if (failed || completed) {
            state = AssetState::Ready;
        }
    }

    void do_unload() override {
        if (!res.empty()) {
            res->unload();
            res.reset(nullptr);
        }
    }

    Res<audio::AudioResource> res;
    String path_;
    String fullPath_;
    bool streaming = false;
};

class AtlasAsset : public Asset {
public:
    AtlasAsset(const char* name, uint32_t formatMask_) : res{name} {
        // we need to load atlas image and atlas meta
        weight_ = 2;
        formatMask = formatMask_;
    }

    void load() override {
        if (state != AssetState::Ready || loaded_scale_ != manager_->scale_uid) {
            loaded_scale_ = manager_->scale_uid;

            res.reset(nullptr);

            fullPath_ = manager_->base_path / res.getID();

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
                int loading = res->getLoadingTexturesCount();
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
                for (auto& page: res->pages) {
                    if (!page.empty()) {
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
    uint8_t loaded_scale_ = 0;
    String fullPath_;
    uint32_t formatMask = 1;
};

class DynamicAtlasAsset : public Asset {
public:
    DynamicAtlasAsset(const char* name, uint32_t flags) : res{name}, flags_{flags} {
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
    explicit SceneAsset(const char* name, String path) :
            res{name},
            path_{std::move(path)} {
    }

    void do_load() override {
        fullPath_ = manager_->base_path / path_;
        get_resource_content_async(
                fullPath_.c_str(),
                [this](auto lr) {
                    if (lr.success()) {
                        res.reset(sg_load(lr.buffer, static_cast<uint32_t>(lr.length)));
                    }
                    lr.close();
                    state = AssetState::Ready;
                });
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
    BitmapFontAsset(const char* name, String path) :
            res{name},
            path_{std::move(path)} {
    }

    void do_load() override {
        fullPath_ = manager_->base_path / path_;
        get_resource_content_async(fullPath_.c_str(), [this](auto lr) {
            if (lr.success()) {
                auto* bmFont = new BitmapFont();
                bmFont->load(lr.buffer, lr.length);
                res.reset(new Font(bmFont));
            }
            lr.close();
            state = AssetState::Ready;
        });
    }

    void do_unload() override {
        res.reset(nullptr);
    }

    Res<Font> res;
    String path_;
    String fullPath_;
};

class TextureAsset : public Asset {
public:
    TextureAsset(const char* name, TextureData data) :
            res{name},
            data_{std::move(data)} {
        weight_ = (float) data_.images.size();
    }

    void do_load() override {
        loader = new TextureLoader();
        loader->basePath = manager_->base_path.c_str();
        EK_ASSERT(data_.images.size() <= TextureLoader::IMAGES_MAX_COUNT);
        for (int i = 0; i < data_.images.size(); ++i) {
            loader->urls[i] = data_.images[i];
        }
        loader->imagesToLoad = (int) data_.images.size();
        if (data_.type == TextureDataType::CubeMap) {
            loader->isCubeMap = true;
            loader->premultiplyAlpha = false;
            loader->formatMask = data_.formatMask;
        }
        loader->load();
        state = AssetState::Loading;
    }

    void poll() override {
        if (loader) {
            if (loader->loading) {
                loader->update();
            }

            if (!loader->loading) {
                error = loader->status;
                if (error == 0) {
                    res.reset(new graphics::Texture{loader->image, loader->desc});
                }
                state = AssetState::Ready;
                delete loader;
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
        res.reset(nullptr);
    }

    Res<Texture> res;
    TextureLoader* loader = nullptr;
    TextureData data_{};
    // by default always premultiply alpha,
    // currently for cube maps will be disabled
    // TODO: export level option
    bool premultiplyAlpha = true;
};

class StringsAsset : public Asset {
public:
    StringsAsset(String name, Array<String> langs) :
            name_{std::move(name)},
            langs_{std::move(langs)} {
        weight_ = (float) langs_.size();
    }

    void do_load() override {
        loaded = 0;
        for (const auto& lang: langs_) {
            auto langPath = manager_->base_path / name_ / lang + ".mo";
            get_resource_content_async(langPath.c_str(),
                                       [this, lang](auto lr) {
                                           if (lr.success()) {
                                               Localization::instance.load(lang.c_str(), lr);
                                           } else {
                                               EK_ERROR("Strings resource not found: %s", lang.c_str());
                                               error = 1;
                                           }
                                           ++loaded;
                                           if (loaded >= langs_.size()) {
                                               state = AssetState::Ready;
                                           }
                                       });
        }
    }

    void do_unload() override {

    }

    String name_;
    Array<String> langs_;
    uint32_t loaded = 0;
};

class ModelAsset : public Asset {
public:
    explicit ModelAsset(String name) :
            name_{std::move(name)} {
    }

    void do_load() override {
        fullPath_ = manager_->base_path / name_ + ".model";
        get_resource_content_async(fullPath_.c_str(), [this](LocalResource lr) {
            if (lr.success()) {
                input_memory_stream input{lr.buffer, lr.length};
                IO io{input};
                Model3D model;
                io(model);
                Res<StaticMesh>{name_.c_str()}.reset(new StaticMesh(model));
                lr.close();
            } else {
                EK_ERROR("MODEL resource not found: %s", fullPath_.c_str());
                error = 1;
            }
            state = AssetState::Ready;
        });
    }

    void do_unload() override {
        Res<StaticMesh>{name_.c_str()}.reset(nullptr);
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
        get_resource_content_async(fullPath_.c_str(), [this](LocalResource lr) {
            if (lr.success()) {
                input_memory_stream input{lr.buffer, lr.length};
                IO io{input};
                bool end = false;
                while (!end) {
                    uint32_t headerSize = 0;
                    io(headerSize);
                    if (headerSize != 0) {
                        auto* asset = manager_->add_from_type(io.stream.dataAtPosition(), headerSize);
                        if (asset) {
                            assets.push_back(asset);
                        }
                        io.stream.seek((int32_t) headerSize);
                    } else {
                        end = true;
                    }
                }
                lr.close();
            }
            // ready for loading
            assetListLoaded = true;
        });
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

    TrueTypeFontAsset(const char* name, String path, String glyphCache, float baseFontSize) :
            res{name},
            baseFontSize_{baseFontSize},
            path_{std::move(path)},
            glyphCache_{std::move(glyphCache)} {
    }

    void do_load() override {
        fullPath_ = manager_->base_path / path_;
        get_resource_content_async(fullPath_.c_str(),
                                   [this](LocalResource lr) {
                                       auto* ttfFont = new TrueTypeFont(manager_->scale_factor, this->baseFontSize_,
                                                                        this->glyphCache_.c_str());
                                       ttfFont->loadFromMemory(lr);
                                       res.reset(new Font(ttfFont));
                                       state = AssetState::Ready;
                                   });
    }

    void do_unload() override {
        res.reset(nullptr);
    }

    Res<Font> res;
    float baseFontSize_;
    String path_;
    String fullPath_;
    String glyphCache_;
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

Asset* DefaultAssetsResolver::create_for_type(const void* data, uint32_t size) const {
    input_memory_stream stream{data, size};
    IO io{stream};
    String type;
    io(type);
    if (type == "audio") {
        String name;
        String path;
        uint32_t flags = 0;
        io(name, path, flags);
        return new AudioAsset(name.c_str(), path, flags);
    } else if (type == "scene") {
        String name;
        String path;
        io(name, path);
        return new SceneAsset(name.c_str(), path);
    } else if (type == "bmfont") {
        String name;
        String path;
        io(name, path);
        return new BitmapFontAsset(name.c_str(), path);
    } else if (type == "ttf") {
        String name;
        String path;
        String glyphCache;
        float baseFontSize;
        io(name, path, glyphCache, baseFontSize);
        return new TrueTypeFontAsset(name.c_str(), path, glyphCache, baseFontSize);
    } else if (type == "atlas") {
        String name;
        uint32_t formatMask = 1;
        io(name, formatMask);
        return new AtlasAsset(name.c_str(), formatMask);
    } else if (type == "dynamic_atlas") {
        String name;
        uint32_t flags;
        io(name, flags);
        return new DynamicAtlasAsset(name.c_str(), flags);
    } else if (type == "model") {
        String name;
        io(name);
        return new ModelAsset(name);
    } else if (type == "texture") {
        String name;
        TextureData texData;
        io(name, texData);
        return new TextureAsset(name.c_str(), texData);
    } else if (type == "strings") {
        String name;
        Array<String> langs;
        io(name, langs);
        return new StringsAsset(name, langs);
    } else if (type == "pack") {
        String name;
        io(name);
        return new PackAsset(name);
    }
    return nullptr;
}

}