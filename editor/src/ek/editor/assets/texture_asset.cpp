#include "texture_asset.hpp"

#include <ek/fs/path.hpp>
#include <scenex/asset2/asset_manager.hpp>
#include <ek/system/system.hpp>
#include <ek/editor/imgui/imgui.hpp>
#include <ek/logger.hpp>
#include <ek/assets.hpp>
#include <graphics/program.hpp>
#include <graphics/vertex_decl.hpp>

#include <pugixml.hpp>
#include <ek/editor/gui/editor_widgets.hpp>
#include <graphics/texture.hpp>
#include <utils/image_loader.hpp>
#include <scenex/data/texture_data.hpp>

using scenex::asset_object_t;

namespace ek {

texture_asset_t::texture_asset_t(std::string path)
        : path_{std::move(path)} {
}

void texture_asset_t::load() {
    pugi::xml_document xml;

    const auto full_path = project_->base_path / path_;
    if (xml.load_file(full_path.c_str())) {
        auto node = xml.first_child();
        name_ = node.attribute("name").as_string();
        texture_type_ = node.attribute("texture_type").as_string("2d");
        for (auto image_node : node.children("image")) {
            images_.emplace_back(image_node.attribute("path").as_string());
        }
    } else {
        EK_ERROR << "error parse xml " << full_path;
    }

    texture_t* texture = nullptr;

    if (texture_type_ == "cubemap") {
        if (images_.size() != 6) {
            EK_ERROR << "Cubemap requires 6 images";
        }
        std::array<image_t*, 6> images{};
        for (size_t i = 0; i < 6; ++i) {
            images[i] = load_image(get_relative_path(path_t{images_[i]}));
        }
        texture = new texture_t(true);
        texture->upload_cubemap(images);
        // +X : right
        // -X : left
        // +Y : top
        // -Y : bottom
        // +Z : front
        // -Z : back

        for (size_t i = 0; i < 6; ++i) {
            delete images[i];
        }
    } else if (texture_type_ == "2d") {
        if (images_.size() != 1) {
            EK_ERROR << "2d texture requires Single image";
        }
        auto* img = load_image(get_relative_path(path_t{images_[0]}));
        if (img) {
            texture = new texture_t(true);
            texture->upload(*img);
            delete img;
        }
    } else {
        EK_ERROR << "Unknown Texture-type " << texture_type_;
    }

    asset_t<texture_t>{name_}.reset(texture);
}

void texture_asset_t::unload() {
    asset_t<program_t>{name_}.reset(nullptr);
    images_.clear();
    texture_type_.clear();
}

void texture_asset_t::gui() {
    if (ImGui::TreeNode(this, "%s (Texture Asset)", path_.c_str())) {
        ImGui::LabelText("Name", "%s", name_.c_str());
        ImGui::LabelText("Texture Type", "%s", texture_type_.c_str());
        gui_asset_object_controls(this);
        for (const auto& image_path : images_) {
            ImGui::LabelText("Image", "%s", image_path.c_str());
        }
        ImGui::TreePop();
    }
}

void texture_asset_t::export_() {
    auto output_path = project_->export_path / name_;
    scenex::texture_data_t data{};
    data.texture_type = texture_type_;
    data.images = images_;

    make_dirs(output_path.dir());

    for (const auto& image_path : images_) {
        copy_file(get_relative_path(path_t{image_path}), project_->export_path / image_path);
    }

    output_memory_stream out{100};
    IO io{out};
    io(data);
    ::ek::save(out, output_path + ".texture");
}

void texture_asset_t::save() {
    // TODO:
}

void texture_asset_t::export_meta(output_memory_stream& output) {
    IO io{output};
    std::string type_name{"texture"};
    io(type_name, name_);
}

path_t texture_asset_t::get_relative_path(const path_t& path) const {
    return (project_->base_path / path_).dir() / path;
}

}