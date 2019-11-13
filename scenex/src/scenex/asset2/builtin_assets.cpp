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
#include <ek/serialize/serialize.hpp>
#include <scenex/data/texture_data.hpp>
#include <utils/image_loader.hpp>
#include <scenex/3d/static_mesh.hpp>
#include <scenex/data/model_data.hpp>

namespace scenex {

class builtin_asset_t : public asset_object_t {
public:
    explicit builtin_asset_t(std::string path)
            : path_(std::move(path)),
              ready_{false} {

    }

    virtual bool do_load() { return false; }

    virtual bool do_unload() { return false; }

    void load() override {
        if (!ready_) {
            ready_ = this->do_load();
        }
    }

    void unload() override {
        if (!ready_) {
            ready_ = !this->do_unload();
        }
    }

    void gui() override {}

    void save() override {}

    void export_() override {}

    std::string path_;
    bool ready_ = false;
};


void reload_atlas(const ek::path_t& base_path, const std::string& name, float scale_factor) {
    ek::asset_t<atlas_t> asset{name};
    asset.reset(nullptr);
    asset.reset(load_atlas((base_path / name).c_str(), scale_factor));
}

class music_asset_t : public builtin_asset_t {
public:
    explicit music_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    bool do_load() override {
        auto& audio = ek::resolve<ek::AudioMini>();
        audio.create_music((project_->base_path / path_).c_str());
        return true;
    }

};

class sound_asset_t : public builtin_asset_t {
public:
    explicit sound_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    bool do_load() override {
        auto& audio = ek::resolve<ek::AudioMini>();
        audio.create_sound((project_->base_path / path_).c_str());
        return true;
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
            reload_atlas(project_->base_path, path_, project_->scale_factor);
            ready_ = true;
        }
    }

    bool do_unload() override {
        ek::asset_t<atlas_t>{path_}.reset(nullptr);
        return true;
    }

    uint8_t loaded_scale_ = 0;
};

class scene_asset_t : public builtin_asset_t {
public:
    explicit scene_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    bool do_load() override {
        ek::asset_t<sg_file>{path_}.reset(sg_load((project_->base_path / path_ + ".sg").str()));
        return true;
    }

    bool do_unload() override {
        ek::asset_t<sg_file>{path_}.reset(nullptr);
        return true;
    }

};

class font_asset_t : public builtin_asset_t {
public:
    explicit font_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    bool do_load() override {
        ek::asset_t<font_t>{path_}.reset(load_font((project_->base_path / path_ + ".font").str()));
        return true;
    }

    bool do_unload() override {
        ek::asset_t<font_t>{path_}.reset(nullptr);
        return false;
    }
};

class program_asset_t : public builtin_asset_t {
public:
    explicit program_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    bool do_load() override {
        const auto full_path = project_->base_path / path_ + ".program";
        auto buffer = ek::get_resource_content(full_path.c_str());
        if (buffer.empty()) {
            EK_ERROR << "PROGRAM resource not found: " << full_path;
            return false;
        }

        EK_DEBUG << "PROGRAM loading: " << full_path;

        ek::input_memory_stream input{buffer.data(), buffer.size()};
        ek::IO io{input};
        program_data_t data;
        io(data);

        if (data.vertex_shader.empty()) {
            EK_ERROR << "empty Vertex Shader: " << path_;
        }
        if (data.fragment_shader.empty()) {
            EK_ERROR << "empty Fragment Shader: " << path_;
        }
        auto* program = new ek::program_t(data.vertex_shader.c_str(), data.fragment_shader.c_str());
        if (data.vertex_layout == "2d") {
            program->vertex = &ek::vertex_2d::decl;
        } else if (data.vertex_layout == "3d") {
            program->vertex = &ek::vertex_3d::decl;
        } else {
            EK_ERROR << "unknown Vertex Layout: " << data.vertex_layout;
        }

        ek::asset_t<ek::program_t>{path_}.reset(program);
        return true;
    }

    bool do_unload() override {
        ek::asset_t<ek::program_t>{path_}.reset(nullptr);
        return true;
    }
};

class texture_asset_t : public builtin_asset_t {
public:
    explicit texture_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    bool do_load() override {
        const auto full_path = project_->base_path / path_ + ".texture";
        auto buffer = ek::get_resource_content(full_path.c_str());
        if (buffer.empty()) {
            EK_ERROR << "TEXTURE resource not found: " << full_path;
            return false;
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
                return false;
            }
        } else {
            EK_ERROR << "unknown Texture Type " << data.texture_type;
            return false;
        }

        ek::asset_t<ek::texture_t>{path_}.reset(texture);
        return true;
    }

    bool do_unload() override {
        ek::asset_t<ek::texture_t>{path_}.reset(nullptr);
        return true;
    }
};

class model_asset_t : public builtin_asset_t {
public:
    explicit model_asset_t(std::string path)
            : builtin_asset_t(std::move(path)) {
    }

    bool do_load() override {
        const auto full_path = project_->base_path / path_ + ".model";
        auto buffer = ek::get_resource_content(full_path.str());
        if (buffer.empty()) {
            EK_ERROR << "MODEL resource not found: " << full_path;
            return false;
        }

        EK_DEBUG << "MODEL loading: " << full_path;

        ek::input_memory_stream input{buffer.data(), buffer.size()};
        ek::IO io{input};
        model_data_t data;
        io(data);

        ek::asset_t<static_mesh_t>{path_}.reset(new static_mesh_t(data.mesh));
        return true;
    }

    bool do_unload() override {
        ek::asset_t<static_mesh_t>{path_}.reset(nullptr);
        return true;
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
    }
    return nullptr;
}

void load_asset_pack(asset_manager_t& assets) {
    using namespace ek;
    auto buffer = get_resource_content((assets.base_path / "pack_meta").str());
    input_memory_stream input{buffer.data(), buffer.size()};
    IO io{input};

    while (true) {
        std::string type;
        std::string path;
        io(type, path);
        if (!type.empty() && !path.empty()) {
            assets.add_from_type(type, path);
        } else {
            break;
        }
    }
}

}
