#include "texture_asset.hpp"

#include <ek/debug.hpp>
#include <ek/scenex/data/texture_data.hpp>
#include <ek/system/system.hpp>
#include <ek/imaging/decoder.hpp>
#include <ek/imaging/image.hpp>

namespace ek {

image_t* load_image(const path_t& path) {
    image_t* image = nullptr;
    auto buffer = read_file(path);
    if (buffer.empty()) {
        EK_WARN("IMAGE resource not found: %s", path.c_str());
    } else {
        image = decode_image_data(buffer.data(), buffer.size());
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
}