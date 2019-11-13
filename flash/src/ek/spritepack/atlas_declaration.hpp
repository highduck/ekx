#pragma once

#include <vector>
#include <string>
#include <ek/math/vec.hpp>
#include <pugixml.hpp>

namespace ek {

struct atlas_resolution_decl_t {
    float scale = 1.0f;
    int2 max_size{2048, 2048};
};

struct atlas_decl_t {
    std::string name;
    std::vector<atlas_resolution_decl_t> resolutions;
};

void from_xml(pugi::xml_node node, atlas_decl_t& atlas);

void to_xml(pugi::xml_node node, const atlas_decl_t& atlas);

}

