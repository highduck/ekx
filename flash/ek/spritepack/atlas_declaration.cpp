#include "atlas_declaration.hpp"

namespace ek {

void from_xml(const pugi::xml_node node, atlas_decl_t& atlas) {
    atlas.name = node.attribute("name").as_string();
    for (auto& resolution_node: node.children("resolution")) {
        atlas_resolution_decl_t res{};
        res.scale = resolution_node.attribute("scale").as_float(res.scale);
        res.max_size.x = resolution_node.attribute("max_width").as_float(res.max_size.x);
        res.max_size.y = resolution_node.attribute("max_height").as_float(res.max_size.y);
        atlas.resolutions.push_back(res);
    }
}

void to_xml(pugi::xml_node node, const atlas_decl_t& atlas) {
    node.append_attribute("name").set_value(atlas.name.c_str());
    for (const auto& resolution: atlas.resolutions) {
        auto resolution_node = node.append_child("resolution");
        resolution_node.append_attribute("scale").set_value(resolution.scale);
        resolution_node.append_attribute("max_width").set_value(resolution.max_size.x);
        resolution_node.append_attribute("max_height").set_value(resolution.max_size.y);
    }
}

}