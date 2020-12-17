#include "builtin_assets.hpp"
#include "asset_manager.hpp"
#include <ek/util/path.hpp>
#include <ek/util/logger.hpp>
#include <ek/audio/audio.hpp>
#include <ek/app/res.hpp>
#include <ek/graphics/program.hpp>
#include <ek/graphics/texture.hpp>
#include <ek/graphics/vertex_decl.hpp>
#include <ek/scenex/data/SGFile.hpp>
#include <ek/scenex/2d/Atlas.hpp>
#include <ek/scenex/data/program_data.hpp>
#include <ek/scenex/data/texture_data.hpp>
#include <ek/scenex/3d/static_mesh.hpp>
#include <ek/scenex/data/model_data.hpp>

#include <ek/scenex/text/Font.hpp>
#include <ek/scenex/text/TrueTypeFont.hpp>
#include <ek/scenex/text/BitmapFont.hpp>

#include <utility>
#include <ek/scenex/2d/DynamicAtlas.hpp>
#include <ek/timers.hpp>
#include <ek/Localization.hpp>

namespace ek {

using graphics::texture_t;
using graphics::program_t;
using graphics::vertex_2d;
using graphics::vertex_3d;

class builtin_asset_t : public asset_object_t {
public:
    explicit builtin_asset_t(std::string path): path_(std::move(path)) {
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
        music->load((project_->base_path / path_).c_str());
        Res<audio::Music>{name_}.reset(music);
        state = AssetObjectState::Ready;
    }

    std::string name_;
};

class sound_asset_t : public builtin_asset_t {
public:
    explicit sound_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {

        name_ = path_;
        // remove extension
        if (name_.size() > 4) {
            name_.erase(name_.size() - 4, 4);
        }
    }

    void do_load() override {
        auto* sound = new audio::Sound();
        // TODO: async
        sound->load((project_->base_path / path_).c_str());
        Res<audio::Sound>{name_}.reset(sound);
        state = AssetObjectState::Ready;
    }

    void do_unload() override {
        Res<Atlas>{path_}.reset(nullptr);
    }

    std::string name_;
};

class atlas_asset_t : public builtin_asset_t {
public:
    explicit atlas_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {
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

class program_asset_t : public builtin_asset_t {
public:
    explicit program_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    void do_load() override {
        const auto full_path = project_->base_path / path_ + ".program";
        get_resource_content_async(full_path.c_str(), [this, full_path](auto buffer) {
            if (buffer.empty()) {
                EK_ERROR << "PROGRAM resource not found: " << full_path;
                return;
            }

            EK_DEBUG << "PROGRAM loading: " << full_path;

            input_memory_stream input{buffer.data(), buffer.size()};
            IO io{input};
            program_data_t data;
            io(data);

            if (data.vertex_shader.empty()) {
                EK_ERROR << "empty Vertex Shader: " << path_;
            }
            if (data.fragment_shader.empty()) {
                EK_ERROR << "empty Fragment Shader: " << path_;
            }
            auto* program = new program_t(data.vertex_shader.c_str(), data.fragment_shader.c_str());
            if (data.vertex_layout == "2d") {
                program->vertex = &vertex_2d::decl;
            } else if (data.vertex_layout == "3d") {
                program->vertex = &vertex_3d::decl;
            } else {
                EK_ERROR << "unknown Vertex Layout: " << data.vertex_layout;
            }

            Res<program_t>{path_}.reset(program);
            state = AssetObjectState::Ready;
        });
    }

    void do_unload() override {
        Res<program_t>{path_}.reset(nullptr);
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
            texture_data_t data;
            io(data);

            texture_t* texture = nullptr;
            if (data.texture_type == "cubemap") {
                texture = new texture_t(true);
                texture->reset(1, 1);
                std::vector<std::string> path_list = data.images;
                for (auto& p : path_list) {
                    p = (project_->base_path / p).str();
                }
                load_texture_cube_lazy(path_list, texture);
            } else if (data.texture_type == "2d") {
                texture = new texture_t();
                texture->reset(1, 1);
                load_texture_lazy((project_->base_path / data.images[0]).c_str(), texture);
            } else {
                EK_ERROR << "unknown Texture Type " << data.texture_type;
                error = 1;
            }

            Res<texture_t>{path_}.reset(texture);
            state = AssetObjectState::Ready;
        });
    }

    void do_unload() override {
        Res<texture_t>{path_}.reset(nullptr);
    }
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
        Res<texture_t>{path_}.reset(nullptr);
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
            model_data_t data;
            io(data);

            Res<static_mesh_t>{path_}.reset(new static_mesh_t(data.mesh));
            state = AssetObjectState::Ready;
        });
    }

    void do_unload() override {
        Res<static_mesh_t>{path_}.reset(nullptr);
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
            currentAssetLoading = 0;
        });
    }

    void do_unload() override {
        for (auto* asset : assets) {
            asset->unload();
        }
        assets.clear();
        currentAssetLoading = -1;
    }

    void poll() override {
        if (state == AssetObjectState::Loading && currentAssetLoading >= 0) {
            timer_t timer;
            while (true) {
                auto* asset = assets[currentAssetLoading];
                if (asset->state == AssetObjectState::Initial) {
                    asset->load();
                } else if (asset->state == AssetObjectState::Loading) {
                    asset->poll();
                } else if (asset->state == AssetObjectState::Ready) {
                    ++currentAssetLoading;
                    if (currentAssetLoading >= assets.size()) {
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
                    for (auto* asset : assets) {
                        acc += asset->getProgress();
                        ++total;
                    }
                    return acc / total;
                }
        }
        return 0.0f;
    }

    int currentAssetLoading = -1;
    std::vector<asset_object_t*> assets;
};


class TrueTypeFontAsset : public builtin_asset_t {
public:
    explicit TrueTypeFontAsset(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    void do_load() override {
        get_resource_content_async((project_->base_path / path_ + ".ttf_settings").c_str(), [this](auto buffer) {
            input_memory_stream input{buffer.data(), buffer.size()};
            IO io{input};
            float fontSize = 48.0f;
            std::string glyphCache = "default_glyph_cache";
            io(fontSize, glyphCache);

            get_resource_content_async((project_->base_path / path_ + ".ttf").c_str(), [&, this](auto buffer) {
                auto* ttfFont = new TrueTypeFont(project_->scale_factor, fontSize, glyphCache);
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
    // todo : potentially could be resolved by file extension
    return nullptr;
}

asset_object_t* builtin_asset_resolver_t::create(const std::string& path) const {
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
    } else if (type == "program") {
        return new program_asset_t(path);
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