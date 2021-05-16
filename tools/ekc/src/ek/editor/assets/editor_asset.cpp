#include "editor_asset.hpp"
#include <ek/serialize/serialize.hpp>
#include <ek/debug.hpp>

namespace ek {

void assets_build_struct_t::meta(std::string type, std::string path) {
    if (pack_meta_stream) {
        IO io{*pack_meta_stream};
        io(type, path);
    }
}

std::string get_default_name(const path_t& p) {
    std::string name = p.basename().str();
    const std::string ext{".asset.xml"};
    const auto i = name.find_last_of(ext);
    if (i != std::string::npos) {
        name = name.substr(0, i - ext.size() + 1);
    }
    return name;
}

editor_asset_t::editor_asset_t(path_t path, std::string type_name)
        : declaration_path_{std::move(path)},
          type_name_{std::move(type_name)} {
}

void editor_asset_t::read_decl() {
    pugi::xml_document xml;
    const auto full_path = project->base_path / declaration_path_;
    if (xml.load_file(full_path.c_str())) {
        auto node = xml.first_child();
        auto default_name = get_default_name(declaration_path_);
        name_ = node.attribute("name").as_string(default_name.c_str());
        resource_path_ = path_t{node.attribute("path").as_string(default_name.c_str())};
        dev_ = node.attribute("dev").as_bool(false);
        read_decl_from_xml(node);
    } else {
        EK_ERROR("error parse xml %s", full_path.c_str());
    }
}

void editor_asset_t::read_decl_from_xml(const pugi::xml_node& node) {
}

path_t editor_asset_t::get_relative_path(const path_t& path) const {
    return (project->base_path / declaration_path_).dir() / path;
}

bool editor_asset_t::isDev() const {
    return dev_;
}

}