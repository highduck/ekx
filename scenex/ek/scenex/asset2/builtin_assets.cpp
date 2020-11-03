#include "builtin_assets.hpp"
#include "asset_manager.hpp"
#include <ek/util/path.hpp>
#include <ek/util/logger.hpp>
#include <ek/audio/audio.hpp>
#include <ek/app/res.hpp>
#include <ek/graphics/program.hpp>
#include <ek/graphics/texture.hpp>
#include <ek/graphics/vertex_decl.hpp>
#include <ek/scenex/data/sg_data.hpp>
#include <ek/scenex/data/sg_factory.hpp>
#include <ek/scenex/2d/atlas.hpp>
#include <ek/scenex/2d/font.hpp>
#include <ek/scenex/data/program_data.hpp>
#include <ek/scenex/data/texture_data.hpp>
#include <ek/scenex/3d/static_mesh.hpp>
#include <ek/scenex/data/model_data.hpp>

#include <utility>
#include <ek/rtfont/ttf_font.hpp>
#include <ek/scenex/2d/bitmap_font.hpp>

namespace ek {

using graphics::texture_t;
using graphics::program_t;
using graphics::vertex_2d;
using graphics::vertex_3d;

class builtin_asset_t : public asset_object_t {
public:
    explicit builtin_asset_t(std::string path)
            : path_(std::move(path)) {

    }

    virtual void do_load() {}

    virtual void do_unload() {}

    void load() override {
        if (!ready_) {
            this->do_load();
        }
    }

    void unload() override {
        if (ready_) {
            this->do_unload();
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
        // TODO: async
        music->load((project_->base_path / path_).c_str());
        asset_t<audio::Music>{name_}.reset(music);
        ready_ = true;
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
        asset_t<audio::Sound>{name_}.reset(sound);
        ready_ = true;
    }

    void do_unload() override {
        asset_t<atlas_t>{path_}.reset(nullptr);
        ready_ = false;
    }

    std::string name_;
};

class atlas_asset_t : public builtin_asset_t {
public:
    explicit atlas_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    void load() override {
        if (!ready_ || loaded_scale_ != project_->scale_uid) {
            loaded_scale_ = project_->scale_uid;

            asset_t<atlas_t>{path_}.reset(nullptr);
            ready_ = false;

            load_atlas((project_->base_path / path_).c_str(), project_->scale_factor, [this](auto* atlas) {
                asset_t<atlas_t>{path_}.reset(atlas);
                ready_ = true;
            });
        }
    }

    void do_unload() override {
        asset_t<atlas_t>{path_}.reset(nullptr);
        ready_ = false;
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
                    asset_t<sg_file>{path_}.reset(sg_load(buffer));
                    ready_ = true;
                });
    }

    void do_unload() override {
        asset_t<sg_file>{path_}.reset(nullptr);
        ready_ = false;
    }

};

class font_asset_t : public builtin_asset_t {
public:
    explicit font_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    void do_load() override {
        get_resource_content_async((project_->base_path / path_ + ".font").c_str(), [this](auto buffer) {
            auto* bmfont = new BitmapFont();
            bmfont->load(buffer);
            asset_t<font_t>{path_}.reset(new font_t(bmfont));
            ready_ = true;
        });
    }

    void do_unload() override {
        asset_t<font_t>{path_}.reset(nullptr);
        ready_ = false;
    }
};

class program_asset_t : public builtin_asset_t {
public:
    explicit program_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    void do_load() override {
        ready_ = false;
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

            asset_t<program_t>{path_}.reset(program);
            ready_ = true;
        });
    }

    void do_unload() override {
        asset_t<program_t>{path_}.reset(nullptr);
        ready_ = false;
    }
};

class texture_asset_t : public builtin_asset_t {
public:
    explicit texture_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    void do_load() override {
        const auto full_path = project_->base_path / path_ + ".texture";
        get_resource_content_async(full_path.c_str(), [this, full_path](auto buffer) {
            if (buffer.empty()) {
                EK_ERROR << "TEXTURE resource not found: " << full_path;
                ready_ = false;
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
                ready_ = false;
            }

            asset_t<texture_t>{path_}.reset(texture);
            ready_ = true;
        });
    }

    void do_unload() override {
        asset_t<texture_t>{path_}.reset(nullptr);
        ready_ = false;
    }
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
                ready_ = false;
            }

            EK_DEBUG << "MODEL loading: " << full_path;

            input_memory_stream input{buffer.data(), buffer.size()};
            IO io{input};
            model_data_t data;
            io(data);

            asset_t<static_mesh_t>{path_}.reset(new static_mesh_t(data.mesh));
            ready_ = true;
        });
    }

    void do_unload() override {
        asset_t<static_mesh_t>{path_}.reset(nullptr);
        ready_ = false;
    }
};

class pack_asset_t : public builtin_asset_t {
public:
    explicit pack_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    void do_load() override {
        get_resource_content_async((project_->base_path / path_).c_str(), [this](auto buffer) {
            input_memory_stream input{buffer.data(), buffer.size()};
            IO io{input};

            while (true) {
                std::string type;
                std::string path;
                io(type, path);
                if (!type.empty() && !path.empty()) {
                    auto* asset = project_->add_from_type(type, path);
                    if (asset) {
                        asset->load();
                    }
                } else {
                    break;
                }
            }
            ready_ = true;
        });
    }

    void do_unload() override {

    }
};


class TTFFontAsset : public builtin_asset_t {
public:
    explicit TTFFontAsset(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    void do_load() override {
        get_resource_content_async((project_->base_path / path_ + ".ttf").c_str(), [this](auto buffer) {
            auto* ttfFont = new TTFFont();
            ttfFont->setScale(project_->scale_factor);
            ttfFont->loadFromMemory(std::move(buffer));
            asset_t<font_t>{path_}.reset(new font_t(ttfFont));
            ready_ = true;
        });
    }

    void do_unload() override {
        asset_t<TTFFont>{path_}.reset(nullptr);
        ready_ = false;
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
    } else if (type == "font") {
        return new font_asset_t(path);
    } else if (type == "ttf") {
        return new TTFFontAsset(path);
    } else if (type == "atlas") {
        return new atlas_asset_t(path);
    } else if (type == "program") {
        return new program_asset_t(path);
    } else if (type == "model") {
        return new model_asset_t(path);
    } else if (type == "texture") {
        return new texture_asset_t(path);
    } else if (type == "pack") {
        return new pack_asset_t(path);
    }
    return nullptr;
}

}