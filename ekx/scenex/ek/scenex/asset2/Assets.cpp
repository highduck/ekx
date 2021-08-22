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



class BuiltinAsset : public Asset {
public:
    explicit BuiltinAsset(std::string path) : path_(std::move(path)) {
    }

    virtual void do_load() {}

    virtual void do_unload() {}

    void load() override {
        if (state == AssetState::Initial) {
            state = AssetState::Loading;
            this->do_load();
        }
    }

    void unload() override {
        if (state == AssetState::Ready) {
            this->do_unload();
            state = AssetState::Initial;
        }
    }

    std::string path_;
};

class AudioAsset : public BuiltinAsset {
public:
    explicit AudioAsset(std::string path, bool streaming_) :
            BuiltinAsset(std::move(path)),
            streaming{streaming_} {
        name_ = path_;
        // remove extension
        if (name_.size() > 4) {
            name_.erase(name_.size() - 4, 4);
        }
    }

    void do_load() override {
        auto* audio = new audio::AudioResource();
        filePath_ = project_->base_path / path_;
        audio->load(filePath_.c_str(), streaming);
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

    std::string name_;
    path_t filePath_;
    bool streaming = false;
};

class AtlasAsset : public BuiltinAsset {
public:
    explicit AtlasAsset(std::string path) :
            BuiltinAsset(std::move(path)) {
    }

    void load() override {
        if (state != AssetState::Ready || loaded_scale_ != project_->scale_uid) {
            loaded_scale_ = project_->scale_uid;

            Res<Atlas>{path_}.reset(nullptr);
            state = AssetState::Loading;

            Atlas::load((project_->base_path / path_).c_str(), project_->scale_factor, [this](auto* atlas) {
                Res<Atlas>{path_}.reset(atlas);
                state = AssetState::Ready;
            });
        }
    }

    void do_unload() override {
        Res<Atlas>{path_}.reset(nullptr);
    }

    uint8_t loaded_scale_ = 0;
};

class DynamicAtlasAsset : public BuiltinAsset {
public:
    explicit DynamicAtlasAsset(std::string path)
            : BuiltinAsset(std::move(path)) {
    }

    // do not reload dynamic atlas, because references to texture* should be invalidated,
    // but current strategy not allow that
    void do_load() override {
        get_resource_content_async(
                (project_->base_path / path_ + ".dynamic_atlas").c_str(),
                [this](auto buffer) {
                    loaded_scale_ = project_->scale_uid;
                    const int pageSize = DynamicAtlas::estimateBetterSize(project_->scale_factor,
                                                                          512,
                                                                          2048);

                    input_memory_stream input{buffer.data(), buffer.size()};
                    IO io{input};
                    bool alphaMap = false;
                    bool mipmaps = false;
                    io(alphaMap, mipmaps);
                    Res<DynamicAtlas>{path_}.reset(new DynamicAtlas(pageSize, pageSize, alphaMap, mipmaps));
                    state = AssetState::Ready;
                });
    }

    void do_unload() override {
        Res<DynamicAtlas>{path_}.reset(nullptr);
    }

    uint8_t loaded_scale_ = 0;
};

class SceneAsset : public BuiltinAsset {
public:
    explicit SceneAsset(std::string path)
            : BuiltinAsset(std::move(path)) {
    }

    void do_load() override {
        get_resource_content_async(
                (project_->base_path / path_ + ".sg").c_str(),
                [this](auto buffer) {
                    Res<SGFile>{path_}.reset(sg_load(buffer.data(), static_cast<uint32_t>(buffer.size())));
                    state = AssetState::Ready;
                });
    }

    void do_unload() override {
        Res<SGFile>{path_}.reset(nullptr);
    }

};

class BitmapFontAsset : public BuiltinAsset {
public:
    explicit BitmapFontAsset(std::string path)
            : BuiltinAsset(std::move(path)) {
    }

    void do_load() override {
        get_resource_content_async((project_->base_path / path_ + ".font").c_str(), [this](auto buffer) {
            auto* bmFont = new BitmapFont();
            bmFont->load(buffer);
            Res<Font>{path_}.reset(new Font(bmFont));
            state = AssetState::Ready;
        });
    }

    void do_unload() override {
        Res<Font>{path_}.reset(nullptr);
    }
};

class TextureAsset : public BuiltinAsset {
public:
    explicit TextureAsset(std::string path) :
            BuiltinAsset(std::move(path)) {
    }

    void do_load() override {
        const auto full_path = project_->base_path / path_ + ".texture";
        get_resource_content_async(full_path.c_str(), [this, full_path](auto buffer) {
            if (buffer.empty()) {
                EK_ERROR << "TEXTURE resource not found: " << full_path;
                error = 1;
            }

            EK_DEBUG << "TEXTURE loading: " << full_path;

            input_memory_stream input{buffer.data(), buffer.size()};
            IO io{input};
            io(data);

            imagesLoaded = 0;

            if (data.type == TextureDataType::Normal) {
                imagePathList[0] = project_->base_path / data.images[0];
                texturesCount = 1;
                state = AssetState::Loading;
            } else if (data.type == TextureDataType::CubeMap) {
                for (int idx = 0; idx < 6; ++idx) {
                    imagePathList[idx] = project_->base_path / data.images[idx];
                    EK_TRACE << "add to loading queue cube-map image #" << idx << ": " << imagePathList[idx];
                }
                texturesCount = 6;
                premultiplyAlpha = false;
                state = AssetState::Loading;
            } else {
                EK_ERROR << "unknown Texture Type " << (uint32_t) data.type;
                error = 1;
                state = AssetState::Ready;
            }
        });
    }

    void poll() override {
        if (texturesCount <= 0) {
            return;
        }
        if (texturesStartLoading < texturesCount) {
            const auto idx = texturesStartLoading++;
            EK_TRACE << "poll loading image #" << idx << " / " << texturesCount;
            get_resource_content_async(
                    imagePathList[idx].c_str(),
                    [this, idx](auto buffer) {
                        ++imagesLoaded;
                        images[idx] = decode_image_data(buffer.data(), buffer.size(), premultiplyAlpha);
                        EK_DEBUG << "texture #" << idx << " loaded " << imagesLoaded << " of " << texturesCount;
                    }
            );
        } else if (imagesLoaded == texturesCount) {
            if (data.type == TextureDataType::Normal) {
                if (images[0]) {
                    Texture* tex = graphics::createTexture(*images[0]);
                    Res<Texture>{path_}.reset(tex);
                    delete images[0];
                } else {
                    error = 1;
                }
            } else if (data.type == TextureDataType::CubeMap) {
                EK_DEBUG << "Cube map images loaded, creating cube texture and cleaning up";
                Res<Texture>{path_}.reset(graphics::createTexture(images));
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
            return (float) imagesLoaded / (float) (texturesCount + 1);
        }
        return Asset::getProgress();
    }

    void do_unload() override {
        Res<Texture>{path_}.reset(nullptr);
    }

    int texturesStartLoading = 0;
    int imagesLoaded = 0;
    int texturesCount = 0;
    path_t imagePathList[6];
    image_t* images[6];
    TextureData data{};
    // by default always premultiply alpha,
    // currently for cube maps will be disabled
    // TODO: export level option
    bool premultiplyAlpha = true;
};

class StringsAsset : public BuiltinAsset {
public:
    explicit StringsAsset(std::string path) :
            BuiltinAsset(std::move(path)) {
    }

    void do_load() override {
        const auto full_path = project_->base_path / path_ / "strings.bin";
        get_resource_content_async(full_path.c_str(), [this, full_path](auto buffer) {
            if (buffer.empty()) {
                EK_ERROR << "Strings resource not found: " << full_path;
                error = 1;
            }

            input_memory_stream input{buffer.data(), buffer.size()};
            IO io{input};
            Array<std::string> langs;
            io(langs);
            num = static_cast<int>(langs.size());
            for (auto& lang : langs) {
                get_resource_content_async(
                        (project_->base_path / path_ / lang + ".mo").c_str(),
                        [this, lang, full_path](std::vector<uint8_t> buffer) {
                            if (buffer.empty()) {
                                EK_ERROR << "Strings resource not found: " << full_path;
                                error = 1;
                            } else {
                                Localization::instance.load(lang, std::move(buffer));
                            }

                            --num;
                            if (num <= 0) {
                                state = AssetState::Ready;
                            }
                        });
            }
        });
    }

    void do_unload() override {
        Res<Texture>{path_}.reset(nullptr);
    }

    int num = 0;
};

class ModelAsset : public BuiltinAsset {
public:
    explicit ModelAsset(std::string path)
            : BuiltinAsset(std::move(path)) {
    }

    void do_load() override {
        const auto full_path = project_->base_path / path_ + ".model";
        get_resource_content_async(full_path.c_str(), [this, full_path](auto buffer) {
            if (buffer.empty()) {
                EK_ERROR << "MODEL resource not found: " << full_path;
                error = 1;
            }

            EK_DEBUG << "MODEL loading: " << full_path;

            input_memory_stream input{buffer.data(), buffer.size()};
            IO io{input};
            Model3D model;
            io(model);

            Res<StaticMesh>{path_}.reset(new StaticMesh(model));
            state = AssetState::Ready;
        });
    }

    void do_unload() override {
        Res<StaticMesh>{path_}.reset(nullptr);
    }
};

class PackAsset : public BuiltinAsset {
public:
    explicit PackAsset(std::string path) :
            BuiltinAsset(std::move(path)) {
    }

    void do_load() override {
        get_resource_content_async((project_->base_path / path_).c_str(), [this](auto buffer) {
            input_memory_stream input{buffer.data(), buffer.size()};
            IO io{input};

            while (true) {
                std::string type;
                std::string path;
                io(type, path);
                if (type.empty() && path.empty()) {
                    // special marker of assets end
                    break;
                } else {
                    auto* asset = project_->add_from_type(type, path);
                    if (asset) {
                        assets.push_back(asset);
                    }
                }
            }
            // ready for loading
            assetsLoaded = 0;
        });
    }

    void do_unload() override {
        for (unsigned i = 0; i < assets._size; ++i) {
            assets._data[i]->unload();
        }
        assets.clear();
        assetsLoaded = 0;
    }

    void poll() override {
        if (state == AssetState::Loading && assetsLoaded >= 0) {
            Stopwatch timer;
            while (assetsLoaded < assets._size) {
                auto* asset = assets._data[assetsLoaded];
                if (asset->state == AssetState::Initial) {
                    EK_DEBUG << "Loading BEGIN: " << ((BuiltinAsset*) asset)->path_;
                    asset->load();
                } else if (asset->state == AssetState::Loading) {
                    asset->poll();
                } else if (asset->state == AssetState::Ready) {
                    EK_DEBUG << "Loading END: " << ((BuiltinAsset*) asset)->path_;
                    ++assetsLoaded;
                    if (assetsLoaded >= assets._size) {
                        state = AssetState::Ready;
                        return;
                    }
                }
                if (timer.readMillis() >= 2) {
                    return;
                }
            }
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
                        acc += assets._data[i]->getProgress();
                        ++total;
                    }
                    return acc / total;
                }
        }
        return 0.0f;
    }

    unsigned assetsLoaded = 0;
    Array<Asset*> assets;
};


class TrueTypeFontAsset : public BuiltinAsset {
public:
    explicit TrueTypeFontAsset(std::string path) :
            BuiltinAsset(std::move(path)) {
    }

    void do_load() override {
        get_resource_content_async((project_->base_path / path_ + ".ttf_settings").c_str(), [this](auto buffer) {
            input_memory_stream input{buffer.data(), buffer.size()};
            IO io{input};
            float fontSize = 48.0f;
            std::string glyphCache = "default_glyph_cache";
            io(fontSize, glyphCache);

            get_resource_content_async((project_->base_path / path_ + ".ttf").c_str(),
                                       [this, glyphCache, fontSize](auto buffer) {
                                           auto* ttfFont = new TrueTypeFont(project_->scale_factor, fontSize,
                                                                            glyphCache);
                                           ttfFont->loadFromMemory(std::move(buffer));
                                           Res<Font>{path_}.reset(new Font(ttfFont));
                                           state = AssetState::Ready;
                                       });
        });
    }

    void do_unload() override {
        Res<TrueTypeFont>{path_}.reset(nullptr);
    }
};

Asset* DefaultAssetsResolver::create_from_file(const std::string& path) const {
    (void) path;
    // todo : potentially could be resolved by file extension
    return nullptr;
}

Asset* DefaultAssetsResolver::create(const std::string& path) const {
    (void) path;
    return nullptr;
}

Asset* DefaultAssetsResolver::create_for_type(const std::string& type, const std::string& path) const {
    if (type == "sound") {
        return new AudioAsset(path, false);
    } else if (type == "music") {
        return new AudioAsset(path, true);
    } else if (type == "scene") {
        return new SceneAsset(path);
    } else if (type == "bmfont") {
        return new BitmapFontAsset(path);
    } else if (type == "ttf") {
        return new TrueTypeFontAsset(path);
    } else if (type == "atlas") {
        return new AtlasAsset(path);
    } else if (type == "dynamic_atlas") {
        return new DynamicAtlasAsset(path);
    } else if (type == "model") {
        return new ModelAsset(path);
    } else if (type == "texture") {
        return new TextureAsset(path);
    } else if (type == "strings") {
        return new StringsAsset(path);
    } else if (type == "pack") {
        return new PackAsset(path);
    }
    return nullptr;
}

}