#include "Asset_impl.hpp"

#include <ek/util/Path.hpp>
#include <ek/debug.hpp>
#include <ek/audio/audio.hpp>
#include <ek/app/res.hpp>

// texture loading
#include <ek/scenex/data/TextureData.hpp>
#include <ek/imaging/image.hpp>
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
#include <ek/Localization.hpp>
#include <ek/ds/Array.hpp>

namespace ek {

using graphics::Texture;
using graphics::Shader;

class AudioAsset : public Asset {
public:
    AudioAsset(std::string name, std::string filepath, uint32_t flags) :
            Asset(std::move(name)),
            path_{std::move(filepath)},
            streaming{flags != 0} {
    }

    void do_load() override {
        auto* audio = new audio::AudioResource();

        fullPath_ = manager_->base_path / path_;
        audio->load(fullPath_.c_str(), streaming);
        Res<audio::AudioResource>{name_}.reset(audio);
    }

    void poll() override {
        auto buffer = Res<audio::AudioResource>{name_}->buffer;
        auto failed = !auph::isActive(buffer.id);
        auto completed = auph::isBufferLoaded(buffer);
        if (failed || completed) {
            state = AssetState::Ready;
        }
    }

    void do_unload() override {
        Res<audio::AudioResource> asset{name_};
        if (!asset.empty()) {
            asset->unload();
            asset.reset(nullptr);
        }
    }

    path_t path_;
    path_t fullPath_;
    bool streaming = false;
};

class AtlasAsset : public Asset {
public:
    explicit AtlasAsset(std::string name) :
            Asset(std::move(name)) {
        // we need to load atlas image and atlas meta
        weight_ = 2;
    }

    void load() override {
        if (state != AssetState::Ready || loaded_scale_ != manager_->scale_uid) {
            loaded_scale_ = manager_->scale_uid;

            Res<Atlas>{name_}.reset(nullptr);
            state = AssetState::Loading;

            fullPath_ = manager_->base_path / name_;
            Atlas::load(fullPath_.c_str(), manager_->scale_factor, [this](auto* atlas) {
                Res<Atlas>{name_}.reset(atlas);
            });
        }
    }

    void poll() override {
        if (state == AssetState::Loading) {
            Res<Atlas> atlas{name_};
            if (!atlas.empty()) {
                int toLoad = (int) atlas->pages.size();
                for (auto& page : atlas->pages) {
                    if (!page.empty()) {
                        --toLoad;
                    }
                }
                if (toLoad == 0) {
                    state = AssetState::Ready;
                }
            }
        }
    }

    [[nodiscard]]
    float getProgress() const override {
        if (state == AssetState::Loading) {
            float progress = 0.0f;
            Res<Atlas> atlas{name_};
            if (!atlas.empty()) {
                progress = 1.0f;
                const auto totalPages = (float) atlas->pages.size();
                float loadedPages = 0.0f;
                for (auto& page : atlas->pages) {
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
        Res<Atlas>{name_}.reset(nullptr);
    }

    uint8_t loaded_scale_ = 0;
    path_t fullPath_;
};

class DynamicAtlasAsset : public Asset {
public:
    DynamicAtlasAsset(std::string name, uint32_t flags) : Asset(std::move(name)), flags_{flags} {
    }

    // do not reload dynamic atlas, because references to texture* should be invalidated,
    // but current strategy not allow that
    void do_load() override {
        const int pageSize = DynamicAtlas::estimateBetterSize(manager_->scale_factor,
                                                              512,
                                                              2048);
        Res<DynamicAtlas>{name_}.reset(
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
        Res<DynamicAtlas>{name_}.reset(nullptr);
    }

    uint32_t flags_ = 0;
};

class SceneAsset : public Asset {
public:
    explicit SceneAsset(std::string name, std::string path) :
            Asset(std::move(name)),
            path_{std::move(path)} {
    }

    void do_load() override {
        fullPath_ = manager_->base_path / path_;
        get_resource_content_async(
                fullPath_.c_str(),
                [this](auto buffer) {
                    Res<SGFile>{name_}.reset(sg_load(buffer.data(), static_cast<uint32_t>(buffer.size())));
                    state = AssetState::Ready;
                });
    }

    void do_unload() override {
        Res<SGFile>{name_}.reset(nullptr);
    }

    path_t path_;
    path_t fullPath_;
};

class BitmapFontAsset : public Asset {
public:
    BitmapFontAsset(std::string name, std::string path) :
            Asset(std::move(name)),
            path_{std::move(path)} {
    }

    void do_load() override {
        fullPath_ = manager_->base_path / path_;
        get_resource_content_async(fullPath_.c_str(), [this](auto buffer) {
            auto* bmFont = new BitmapFont();
            bmFont->load(buffer);
            Res<Font>{name_}.reset(new Font(bmFont));
            state = AssetState::Ready;
        });
    }

    void do_unload() override {
        Res<Font>{name_}.reset(nullptr);
    }

    path_t path_;
    path_t fullPath_;
};

class TextureAsset : public Asset {
public:
    TextureAsset(std::string name, TextureData data) :
            Asset(std::move(name)),
            data_{std::move(data)} {
        weight_ = (float) data_.images.size();
    }

    void do_load() override {
        imagesLoaded = 0;

        if (data_.type == TextureDataType::Normal) {
            imagePathList[0] = manager_->base_path / data_.images[0];
            texturesCount = 1;
            state = AssetState::Loading;
        } else if (data_.type == TextureDataType::CubeMap) {
            for (int idx = 0; idx < 6; ++idx) {
                imagePathList[idx] = manager_->base_path / data_.images[idx];
                EK_TRACE_F("add to loading queue cube-map image #%d: %s", idx, imagePathList[idx].c_str());
            }
            texturesCount = 6;
            premultiplyAlpha = false;
            state = AssetState::Loading;
        } else {
            EK_ERROR_F("unknown Texture Type %u", (uint32_t)data_.type);
            error = 1;
            state = AssetState::Ready;
        }
    }

    void poll() override {
        if (texturesCount <= 0 || state != AssetState::Loading) {
            return;
        }
        if (texturesStartLoading < texturesCount) {
            const auto idx = texturesStartLoading++;
            EK_TRACE_F("poll loading image #%d / %d", idx, texturesCount);
            get_resource_content_async(
                    imagePathList[idx].c_str(),
                    [this, idx](auto buffer) {
                        ++imagesLoaded;
                        images[idx] = decode_image_data(buffer.data(), buffer.size(), premultiplyAlpha);
                        EK_DEBUG_F("texture #%d loaded %d of %d", idx, imagesLoaded, texturesCount);
                    }
            );
        } else if (imagesLoaded == texturesCount) {
            if (data_.type == TextureDataType::Normal) {
                if (images[0]) {
                    Texture* tex = graphics::createTexture(*images[0]);
                    Res<Texture>{name_}.reset(tex);
                    delete images[0];
                } else {
                    error = 1;
                }
            } else if (data_.type == TextureDataType::CubeMap) {
                EK_DEBUG("Cube map images loaded, creating cube texture and cleaning up");
                Res<Texture>{name_}.reset(graphics::createTexture(images));
                state = AssetState::Ready;
                for (auto* img : images) {
                    delete img;
                }
            }
            state = AssetState::Ready;
        }
    }

    [[nodiscard]]
    float getProgress() const override {
        if (state == AssetState::Loading) {
            return (float) imagesLoaded / (float) texturesCount;
        }
        return Asset::getProgress();
    }

    void do_unload() override {
        Res<Texture>{name_}.reset(nullptr);
    }

    int texturesStartLoading = 0;
    int imagesLoaded = 0;
    int texturesCount = 0;
    path_t imagePathList[6];
    image_t* images[6]{};
    TextureData data_{};
    // by default always premultiply alpha,
    // currently for cube maps will be disabled
    // TODO: export level option
    bool premultiplyAlpha = true;
};

class StringsAsset : public Asset {
public:
    StringsAsset(std::string name, Array<std::string> langs) :
            Asset(std::move(name)),
            langs_{std::move(langs)} {
        weight_ = (float) langs_.size();
    }

    void do_load() override {
        loaded = 0;
        for (const auto& lang : langs_) {
            auto langPath = manager_->base_path / name_ / lang + ".mo";
            get_resource_content_async(langPath.c_str(),
                                       [this, lang](std::vector<uint8_t> buffer) {
                                           if (buffer.empty()) {
                                               EK_ERROR_F("Strings resource not found: %s", lang.c_str());
                                               error = 1;
                                           } else {
                                               Localization::instance.load(lang, std::move(buffer));
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

    Array<std::string> langs_;
    int loaded = 0;
};

class ModelAsset : public Asset {
public:
    explicit ModelAsset(std::string name) :
            Asset(std::move(name)) {
    }

    void do_load() override {
        fullPath_ = manager_->base_path / name_ + ".model";
        get_resource_content_async(fullPath_.c_str(), [this](auto buffer) {
            if (buffer.empty()) {
                EK_ERROR_F("MODEL resource not found: %s", fullPath_.c_str());
                error = 1;
            }

            input_memory_stream input{buffer.data(), buffer.size()};
            IO io{input};
            Model3D model;
            io(model);

            Res<StaticMesh>{name_}.reset(new StaticMesh(model));
            state = AssetState::Ready;
        });
    }

    void do_unload() override {
        Res<StaticMesh>{name_}.reset(nullptr);
    }

    path_t fullPath_;
};

bool isTimeBudgetAllowStartNextJob(const Stopwatch& timer) {
    return timer.readMillis() < 8.0f;
}

class PackAsset : public Asset {
public:

    bool assetListLoaded = false;

    explicit PackAsset(std::string name) :
            Asset(std::move(name)) {
    }

    void do_load() override {
        assetListLoaded = false;
        assetsLoaded = 0;
        fullPath_ = manager_->base_path / name_;
        get_resource_content_async(fullPath_.c_str(), [this](auto buffer) {
            input_memory_stream input{buffer.data(), buffer.size()};
            IO io{input};
            bool end = false;
            while (!end) {
                int32_t headerSize = 0;
                io(headerSize);
                if (headerSize != 0) {
                    auto* asset = manager_->add_from_type(io.stream.dataAtPosition(), headerSize);
                    if (asset) {
                        assets.push_back(asset);
                    }
                    io.stream.seek(headerSize);
                } else {
                    end = true;
                }
            }
            // ready for loading
            assetListLoaded = true;
        });
    }

    void do_unload() override {
        for (unsigned i = 0; i < assets._size; ++i) {
            assets._data[i]->unload();
        }
        assets.clear();
        assetsLoaded = 0;
        assetListLoaded = false;
    }

    void poll() override {
        if (state != AssetState::Loading || !assetListLoaded) {
            return;
        }

        Stopwatch timer;
        int numAssetsLoaded = 0;
        for (int i = 0; i < assets._size; ++i) {
            auto* asset = assets._data[i];
            const auto initialState = asset->state;
            if (asset->state == AssetState::Initial) {
                if (isTimeBudgetAllowStartNextJob(timer)) {
                    EK_DEBUG_F("Loading BEGIN: %s", asset->name_.c_str());
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
                    EK_DEBUG_F("Loading END: %s", asset->name_.c_str());
                }
                ++numAssetsLoaded;
            }
        }

        if (!isTimeBudgetAllowStartNextJob(timer)) {
            EK_INFO_F("Assets loading jobs spend %d ms", (int)timer.readMillis());
        }

        assetsLoaded = numAssetsLoaded;
        if (numAssetsLoaded >= assets._size) {
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
                    for (unsigned i = 0; i < assets._size; ++i) {
                        const float w = assets._data[i]->weight_;
                        acc += w * assets._data[i]->getProgress();
                        total += w;
                    }
                    if (total > 0.0f) {
                        return acc / total;
                    }
                }
        }
        return 0.0f;
    }

    unsigned assetsLoaded = 0;
    Array<Asset*> assets;
    path_t fullPath_;
};

class TrueTypeFontAsset : public Asset {
public:

    TrueTypeFontAsset(std::string name, std::string path, std::string glyphCache, float baseFontSize) :
            Asset(std::move(name)),
            path_{std::move(path)},
            glyphCache_{std::move(glyphCache)},
            baseFontSize_{baseFontSize} {
    }

    void do_load() override {
        fullPath_ = manager_->base_path / path_;
        get_resource_content_async(fullPath_.c_str(),
                                   [this](auto buffer) {
                                       auto* ttfFont = new TrueTypeFont(manager_->scale_factor, this->baseFontSize_,
                                                                        this->glyphCache_);
                                       ttfFont->loadFromMemory(std::move(buffer));
                                       Res<Font>{name_}.reset(new Font(ttfFont));
                                       state = AssetState::Ready;
                                   });
    }

    void do_unload() override {
        Res<Font>{name_}.reset(nullptr);
    }

    float baseFontSize_;
    path_t path_;
    path_t fullPath_;
    std::string glyphCache_;
};

Asset* DefaultAssetsResolver::create_from_file(const std::string& path, const std::string& type) const {
    if (type == "pack") {
        return new PackAsset(path);
    }
    return nullptr;
}

Asset* DefaultAssetsResolver::create(const std::string& path) const {
    (void) path;
    return nullptr;
}

Asset* DefaultAssetsResolver::create_for_type(const void* data, int size) const {
    input_memory_stream stream{data, size};
    IO io{stream};
    std::string type;
    io(type);
    if (type == "audio") {
        std::string name;
        std::string path;
        uint32_t flags = 0;
        io(name, path, flags);
        return new AudioAsset(name, path, flags);
    } else if (type == "scene") {
        std::string name;
        std::string path;
        io(name, path);
        return new SceneAsset(name, path);
    } else if (type == "bmfont") {
        std::string name;
        std::string path;
        io(name, path);
        return new BitmapFontAsset(name, path);
    } else if (type == "ttf") {
        std::string name;
        std::string path;
        std::string glyphCache;
        float baseFontSize;
        io(name, path, glyphCache, baseFontSize);
        return new TrueTypeFontAsset(name, path, glyphCache, baseFontSize);
    } else if (type == "atlas") {
        std::string name;
        io(name);
        return new AtlasAsset(name);
    } else if (type == "dynamic_atlas") {
        std::string name;
        uint32_t flags;
        io(name, flags);
        return new DynamicAtlasAsset(name, flags);
    } else if (type == "model") {
        std::string name;
        io(name);
        return new ModelAsset(name);
    } else if (type == "texture") {
        std::string name;
        TextureData texData;
        io(name, texData);
        return new TextureAsset(name, texData);
    } else if (type == "strings") {
        std::string name;
        Array<std::string> langs;
        io(name, langs);
        return new StringsAsset(name, langs);
    } else if (type == "pack") {
        std::string name;
        io(name);
        return new PackAsset(name);
    }
    return nullptr;
}

}