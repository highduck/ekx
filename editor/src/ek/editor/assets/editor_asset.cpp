#include "editor_asset.hpp"
#include <ek/logger.hpp>

namespace ek {

std::string get_default_name(const path_t& p) {
    std::string name = p.basename().str();
    const std::string ext{".asset.xml"};
    const auto i = name.find_last_of(ext);
    if (i != std::string::npos) {
        name = name.substr(0, i - ext.size() + 1);
    }
    return name;
}

editor_asset_t::editor_asset_t(std::string path, std::string type_name)
        : path_{std::move(path)},
          type_name_{std::move(type_name)} {
}

void editor_asset_t::read_decl() {
    pugi::xml_document xml;
    const auto full_path = project_->base_path / path_;
    if (xml.load_file(full_path.c_str())) {
        auto node = xml.first_child();
        auto default_name = get_default_name(path_);
        name_ = node.attribute("name").as_string(default_name.c_str());
        resource_path_ = path_t{node.attribute("path").as_string(default_name.c_str())};
        read_decl_from_xml(node);
    } else {
        EK_ERROR("error parse xml %s", full_path.c_str());
    }
}

void editor_asset_t::read_decl_from_xml(const pugi::xml_node& node) {
}

void editor_asset_t::gui() {
}

void editor_asset_t::export_() {
}

void editor_asset_t::save() {
}

void editor_asset_t::export_meta(output_memory_stream& output) {
//    IO io{output};
//    std::string type_name{"texture"};
//    io(type_name, name_);
}

path_t editor_asset_t::get_relative_path(const path_t& path) const {
    return (project_->base_path / path_).dir() / path;
}

}