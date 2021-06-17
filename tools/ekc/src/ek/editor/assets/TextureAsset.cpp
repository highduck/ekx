#include "TextureAsset.hpp"

#include <ek/debug.hpp>
#include <ek/scenex/data/TextureData.hpp>
#include <ek/system/system.hpp>
#include <ek/imaging/image.hpp>

namespace ek {

image_t* load_image(const path_t& path) {
    image_t* image = nullptr;
    auto buffer = read_file(path);
    if (buffer.empty()) {
        EK_WARN << "IMAGE resource not found: " << path;
    } else {
        image = decode_image_data(buffer.data(), buffer.size());
    }
    return image;
}

TextureAsset::TextureAsset(path_t path) :
        editor_asset_t{std::move(path), "texture"} {
}

void TextureAsset::read_decl_from_xml(const pugi::xml_node& node) {
    data.images.clear();
    std::string type = node.attribute("texture_type").as_string("2d");
    data.type = TextureDataType::Normal;
    if (type == "2d") {
        data.type = TextureDataType::Normal;
    } else if (type == "cubemap") {
        data.type = TextureDataType::CubeMap;
    } else {
        EK_WARN << "Unknown texture type " << type;
    }

    for (auto image_node : node.children("image")) {
        data.images.emplace_back(image_node.attribute("path").as_string());
    }
}

void TextureAsset::build(assets_build_struct_t& build_data) {
    read_decl();

    const auto output_path = build_data.output / name_;

    make_dirs(output_path.dir());

    for (const auto& image_path : data.images) {
        copy_file(get_relative_path(path_t{image_path}), build_data.output / image_path);
    }

    output_memory_stream out{100};
    IO io{out};
    io(data);
    ::ek::save(out, output_path + ".texture");

    build_data.meta(type_name_, name_);
}
}