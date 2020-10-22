#include "texture_asset.hpp"

#include <ek/editor/imgui/imgui.hpp>
#include <ek/util/logger.hpp>
#include <ek/util/assets.hpp>
#include <ek/graphics/program.hpp>
#include <ek/graphics/vertex_decl.hpp>

#include <ek/graphics/texture.hpp>
#include <ek/scenex/data/texture_data.hpp>
#include <ek/system/system.hpp>
#include <ek/imaging/decoder.hpp>

namespace ek {

using graphics::texture_t;
using graphics::program_t;

image_t* load_image(const path_t& path) {
    image_t* image = nullptr;
    auto buffer = read_file(path);
    if (buffer.empty()) {
        EK_WARN("IMAGE resource not found: %s", path.c_str());
    } else {
        image = decode_image_data(buffer);
    }
    return image;
}

texture_asset_t::texture_asset_t(path_t path)
        : editor_asset_t{std::move(path), "texture"} {
}

void texture_asset_t::read_decl_from_xml(const pugi::xml_node& node) {
    images_.clear();
    texture_type_ = node.attribute("texture_type").as_string("2d");
    for (auto image_node : node.children("image")) {
        images_.emplace_back(image_node.attribute("path").as_string());
    }
}

void texture_asset_t::load() {
    read_decl();

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
    ImGui::LabelText("Texture Type", "%s", texture_type_.c_str());
    for (const auto& image_path : images_) {
        ImGui::LabelText("Image", "%s", image_path.c_str());
    }
}

void texture_asset_t::build(assets_build_struct_t& build_data) {
    read_decl();

    const auto output_path = build_data.output / name_;
    texture_data_t data{};
    data.texture_type = texture_type_;
    data.images = images_;

    make_dirs(output_path.dir());

    for (const auto& image_path : images_) {
        copy_file(get_relative_path(path_t{image_path}), build_data.output / image_path);
    }

    output_memory_stream out{100};
    IO io{out};
    io(data);
    ::ek::save(out, output_path + ".texture");

    build_data.meta(type_name_, name_);
}

void texture_asset_t::save() {
    // TODO:
}

}