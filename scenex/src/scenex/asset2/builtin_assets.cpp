#include "builtin_assets.hpp"
#include "asset_manager.hpp"
#include <ek/fs/path.hpp>
#include <scenex/2d/atlas.hpp>
#include <ek/locator.hpp>
#include <ek/audiomini.hpp>
#include <scenex/data/sg_data.h>
#include <scenex/data/sg_factory.h>
#include <scenex/2d/font.hpp>
#include <graphics/program.hpp>
#include <graphics/texture.hpp>
#include <ek/imaging/image.hpp>
#include <utility>
#include <graphics/vertex_decl.hpp>
#include <scenex/data/program_data.hpp>
#include <platform/static_resources.hpp>
#include <ek/logger.hpp>
#include <scenex/data/texture_data.hpp>
#include <utils/image_loader.hpp>
#include <scenex/3d/static_mesh.hpp>
#include <scenex/data/model_data.hpp>
#include <ek/serialize/serialize.hpp>

namespace scenex {

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

    void gui() override {}

    void save() override {}

    void export_() override {}

    std::string path_;
};

class music_asset_t : public builtin_asset_t {
public:
    explicit music_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    void do_load() override {
        auto& audio = ek::resolve<ek::AudioMini>();
        audio.create_music((project_->base_path / path_).c_str());
        ready_ = true;
    }

};

class sound_asset_t : public builtin_asset_t {
public:
    explicit sound_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    void do_load() override {
        auto& audio = ek::resolve<ek::AudioMini>();
        audio.create_sound((project_->base_path / path_).c_str());
        ready_ = true;
    }
};

class atlas_asset_t : public builtin_asset_t {
public:
    explicit atlas_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    void load() override {
        if (!ready_ || loaded_scale_ != project_->scale_uid) {
            loaded_scale_ = project_->scale_uid;

            ek::asset_t<atlas_t>{path_}.reset(nullptr);
            ready_ = false;

            load_atlas((project_->base_path / path_).c_str(), project_->scale_factor, [this](auto* atlas) {
                ek::asset_t<atlas_t>{path_}.reset(atlas);
                ready_ = true;
            });
        }
    }

    void do_unload() override {
        ek::asset_t<atlas_t>{path_}.reset(nullptr);
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
        using namespace ek;
        get_resource_content_async(
                (project_->base_path / path_ + ".sg").c_str(),
                [this](auto buffer) {
                    ek::asset_t<sg_file>{path_}.reset(sg_load(buffer));
                    ready_ = true;
                });
    }

    void do_unload() override {
        ek::asset_t<sg_file>{path_}.reset(nullptr);
        ready_ = false;
    }

};

class font_asset_t : public builtin_asset_t {
public:
    explicit font_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    void do_load() override {
        using namespace ek;
        get_resource_content_async((project_->base_path / path_ + ".font").c_str(), [this](auto buffer) {
            ek::asset_t<font_t>{path_}.reset(load_font(buffer));
            ready_ = true;
        });
    }

    void do_unload() override {
        ek::asset_t<font_t>{path_}.reset(nullptr);
        ready_ = false;
    }
};

class program_asset_t : public builtin_asset_t {
public:
    explicit program_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    void do_load() override {
        using namespace ek;
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
        ek::asset_t<ek::program_t>{path_}.reset(nullptr);
        ready_ = false;
    }
};

class texture_asset_t : public builtin_asset_t {
public:
    explicit texture_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    void do_load() override {
        using namespace ek;
        const auto full_path = project_->base_path / path_ + ".texture";
        get_resource_content_async(full_path.c_str(), [this, full_path](auto buffer) {
            if (buffer.empty()) {
                EK_ERROR << "TEXTURE resource not found: " << full_path;
                ready_ = false;
            }

            EK_DEBUG << "TEXTURE loading: " << full_path;

            ek::input_memory_stream input{buffer.data(), buffer.size()};
            ek::IO io{input};
            texture_data_t data;
            io(data);

            ek::texture_t* texture = nullptr;
            if (data.texture_type == "cubemap") {
                texture = new ek::texture_t(true);
                std::array<ek::image_t*, 6> images{};
                for (size_t i = 0; i < 6; ++i) {
                    images[i] = ek::load_image(project_->base_path / data.images[i]);
                }
                texture->upload_cubemap(images);
                for (auto* img : images) {
                    delete img;
                }
            } else if (data.texture_type == "2d") {
                auto* img = ek::load_image(project_->base_path / data.images[0]);
                if (img) {
                    texture = new ek::texture_t();
                    texture->upload(*img);
                    delete img;
                } else {
                    ready_ = false;
                }
            } else {
                EK_ERROR << "unknown Texture Type " << data.texture_type;
                ready_ = false;
            }

            ek::asset_t<ek::texture_t>{path_}.reset(texture);
            ready_ = true;
        });
    }

    void do_unload() override {
        ek::asset_t<ek::texture_t>{path_}.reset(nullptr);
        ready_ = false;
    }
};

class model_asset_t : public builtin_asset_t {
public:
    explicit model_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    void do_load() override {
        using namespace ek;
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
        ek::asset_t<static_mesh_t>{path_}.reset(nullptr);
        ready_ = false;
    }
};

class pack_asset_t : public builtin_asset_t {
public:
    explicit pack_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    void do_load() override {
        using namespace ek;
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

asset_object_t* builtin_asset_resolver_t::create_from_file(const std::string& path) const {
    // todo : potentially could be resolved by file extension
    return nullptr;
}

scenex::asset_object_t* builtin_asset_resolver_t::create(const std::string& path) const {
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