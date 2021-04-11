#include "builtin_assets.hpp"
#include "asset_manager.hpp"
#include <ek/util/path.hpp>
#include <ek/util/logger.hpp>
#include <ek/audio/audio.hpp>
#include <ek/app/res.hpp>

// texture loading
#include <ek/scenex/data/texture_data.hpp>
#include <ek/imaging/decoder.hpp>
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
#include <ek/timers.hpp>
#include <ek/Localization.hpp>
#include <ek/ds/Array.hpp>

namespace ek {

using graphics::Texture;
using graphics::Shader;

class builtin_asset_t : public asset_object_t {
public:
    explicit builtin_asset_t(std::string path) : path_(std::move(path)) {
    }

    virtual void do_load() {}

    virtual void do_unload() {}

    void load() override {
        if (state == AssetObjectState::Initial) {
            state = AssetObjectState::Loading;
            this->do_load();
        }
    }

    void unload() override {
        if (state == AssetObjectState::Ready) {
            this->do_unload();
            state = AssetObjectState::Initial;
        }
    }

    std::string path_;
};

class music_asset_t : public builtin_asset_t {
public:
    explicit music_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {

        name_ = path_;
        // remove extension
        if (name_.size() > 4) {
            name_.erase(name_.size() - 4, 4);
        }
    }

    void do_load() override {
        auto* music = new audio::Music();
        filePath_ = project_->base_path / path_;
        music->load(filePath_.c_str());
        Res<audio::Music>{name_}.reset(music);
        state = AssetObjectState::Ready;
    }

    std::string name_;
    path_t filePath_;
};

class sound_asset_t : public builtin_asset_t {
public:
    explicit sound_asset_t(std::string path) :
            builtin_asset_t(std::move(path)) {
        name_ = path_;
        // remove extension
        if (name_.size() > 4) {
            name_.erase(name_.size() - 4, 4);
        }
    }

    void do_load() override {
        auto* sound = new audio::Sound();
        // TODO: async
        filePath_ = project_->base_path / path_;
        sound->load(filePath_.c_str());
        Res<audio::Sound>{name_}.reset(sound);
        state = AssetObjectState::Ready;
    }

    void do_unload() override {
        Res<Atlas>{path_}.reset(nullptr);
    }

    std::string name_;
    path_t filePath_;
};

class atlas_asset_t : public builtin_asset_t {
public:
    explicit atlas_asset_t(std::string path) :
            builtin_asset_t(std::move(path)) {
    }

    void load() override {
        if (state != AssetObjectState::Ready || loaded_scale_ != project_->scale_uid) {
            loaded_scale_ = project_->scale_uid;

            Res<Atlas>{path_}.reset(nullptr);
            state = AssetObjectState::Loading;

            Atlas::load((project_->base_path / path_).c_str(), project_->scale_factor, [this](auto* atlas) {
                Res<Atlas>{path_}.reset(atlas);
                state = AssetObjectState::Ready;
            });
        }
    }

    void do_unload() override {
        Res<Atlas>{path_}.reset(nullptr);
    }

    uint8_t loaded_scale_ = 0;
};

class DynamicAtlasAsset : public builtin_asset_t {
public:
    explicit DynamicAtlasAsset(std::string path)
            : builtin_asset_t(std::move(path)) {
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
                    state = AssetObjectState::Ready;
                });
    }

    void do_unload() override {
        Res<DynamicAtlas>{path_}.reset(nullptr);
    }

    uint8_t loaded_scale_ = 0;
};

class scene_asset_t : public builtin_asset_t {
public:
    explicit scene_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    void do_load() override {
        get_resource_content_async(
                (project_->base_path / path_ + ".sg").c_str(),
                [this](auto buffer) {
                    Res<SGFile>{path_}.reset(sg_load(buffer));
                    state = AssetObjectState::Ready;
                });
    }

    void do_unload() override {
        Res<SGFile>{path_}.reset(nullptr);
    }

};

class BitmapFontAsset : public builtin_asset_t {
public:
    explicit BitmapFontAsset(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    void do_load() override {
        get_resource_content_async((project_->base_path / path_ + ".font").c_str(), [this](auto buffer) {
            auto* bmFont = new BitmapFont();
            bmFont->load(buffer);
            Res<Font>{path_}.reset(new Font(bmFont));
            state = AssetObjectState::Ready;
        });
    }

    void do_unload() override {
        Res<Font>{path_}.reset(nullptr);
    }
};

class texture_asset_t : public builtin_asset_t {
public:
    explicit texture_asset_t(std::string path) :
            builtin_asset_t(std::move(path)) {
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

            if (data.texture_type == "cubemap") {
                for (int idx = 0; idx < 6; ++idx) {
                    imagePathList[idx] = project_->base_path / data.images[idx];
                }
                texturesCount = 6;
                premultiplyAlpha = false;
            } else if (data.texture_type == "2d") {
                imagePathList[0] = project_->base_path / data.images[0];
                texturesCount = 1;
            } else {
                EK_ERROR << "unknown Texture Type " << data.texture_type;
                error = 1;
                state = AssetObjectState::Ready;
            }
        });
    }

    void poll() override {
        if (texturesStartLoading < texturesCount) {
            const auto idx = texturesStartLoading++;
            get_resource_content_async(
                    imagePathList[idx].c_str(),
                    [this](auto buffer) {
                        images[imagesLoaded++] = decode_image_data(buffer.data(), buffer.size(), premultiplyAlpha);
                        EK_DEBUG << "textures loading: #" << imagesLoaded << " of " << texturesCount;
                    }
            );
        } else if (imagesLoaded == texturesCount) {
            if (data.texture_type == "cubemap") {
                EK_DEBUG << "Cube map images loaded, creating cube texture and cleaning up";
                Res<Texture>{path_}.reset(graphics::createTexture(images));
                state = AssetObjectState::Ready;
                for (auto* img : images) {
                    delete img;
                }
            } else if (data.texture_type == "2d") {
                if (images[0]) {
                    Texture* tex = graphics::createTexture(*images[0]);
                    Res<Texture>{path_}.reset(tex);
                    delete images[0];
                } else {
                    error = 1;
                }
            }
            state = AssetObjectState::Ready;
        }
    }

    [[nodiscard]]
    float getProgress() const override {
        if (state == AssetObjectState::Loading) {
            return (float) imagesLoaded / (float) (texturesCount + 1);
        }
        return asset_object_t::getProgress();
    }

    void do_unload() override {
        Res<Texture>{path_}.reset(nullptr);
    }

    int texturesStartLoading = 0;
    int imagesLoaded = 0;
    int texturesCount = 0;
    path_t imagePathList[6];
    image_t* images[6];
    texture_data_t data{};
    // by default always premultiply alpha,
    // currently for cube maps will be disabled
    // TODO: export level option
    bool premultiplyAlpha = true;
};

class StringsAsset : public builtin_asset_t {
public:
    explicit StringsAsset(std::string path) :
            builtin_asset_t(std::move(path)) {
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
            std::vector<std::string> langs;
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
                                state = AssetObjectState::Ready;
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

class model_asset_t : public builtin_asset_t {
public:
    explicit model_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {
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
            state = AssetObjectState::Ready;
        });
    }

    void do_unload() override {
        Res<StaticMesh>{path_}.reset(nullptr);
    }
};

class pack_asset_t : public builtin_asset_t {
public:
    explicit pack_asset_t(std::string path) :
            builtin_asset_t(std::move(path)) {
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
        if (state == AssetObjectState::Loading && assetsLoaded >= 0) {
            timer_t timer;
            while (assetsLoaded < assets._size) {
                auto* asset = assets._data[assetsLoaded];
                if (asset->state == AssetObjectState::Initial) {
                    EK_DEBUG << "Loading BEGIN: " << ((builtin_asset_t*) asset)->path_;
                    asset->load();
                } else if (asset->state == AssetObjectState::Loading) {
                    asset->poll();
                } else if (asset->state == AssetObjectState::Ready) {
                    EK_DEBUG << "Loading END: " << ((builtin_asset_t*) asset)->path_;
                    ++assetsLoaded;
                    if (assetsLoaded >= assets._size) {
                        state = AssetObjectState::Ready;
                        return;
                    }
                }
                if (timer.read_millis() >= 2) {
                    return;
                }
            }
        }
    }

    [[nodiscard]] float getProgress() const override {
        switch (state) {
            case AssetObjectState::Ready:
                return 1.0f;
            case AssetObjectState::Initial:
                return 0.0f;
            case AssetObjectState::Loading:
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
    Array<asset_object_t*> assets;
};


class TrueTypeFontAsset : public builtin_asset_t {
public:
    explicit TrueTypeFontAsset(std::string path) :
            builtin_asset_t(std::move(path)) {
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
                                           state = AssetObjectState::Ready;
                                       });
        });
    }

    void do_unload() override {
        Res<TrueTypeFont>{path_}.reset(nullptr);
    }
};

asset_object_t* builtin_asset_resolver_t::create_from_file(const std::string& path) const {
    (void) path;
    // todo : potentially could be resolved by file extension
    return nullptr;
}

asset_object_t* builtin_asset_resolver_t::create(const std::string& path) const {
    (void) path;
    return nullptr;
}

asset_object_t* builtin_asset_resolver_t::create_for_type(const std::string& type, const std::string& path) const {
    if (type == "sound") {
        return new sound_asset_t(path);
    } else if (type == "music") {
        return new music_asset_t(path);
    } else if (type == "scene") {
        return new scene_asset_t(path);
    } else if (type == "bmfont") {
        return new BitmapFontAsset(path);
    } else if (type == "ttf") {
        return new TrueTypeFontAsset(path);
    } else if (type == "atlas") {
        return new atlas_asset_t(path);
    } else if (type == "dynamic_atlas") {
        return new DynamicAtlasAsset(path);
    } else if (type == "model") {
        return new model_asset_t(path);
    } else if (type == "texture") {
        return new texture_asset_t(path);
    } else if (type == "strings") {
        return new StringsAsset(path);
    } else if (type == "pack") {
        return new pack_asset_t(path);
    }
    return nullptr;
}

}