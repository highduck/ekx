#include <ek/utility/strings.hpp>
#include "filters_declaration.hpp"

namespace ek {

filter_type get_filter_type(const std::string& type) {
    if (type == "refill") {
        return filter_type::refill;
    } else if (type == "glow") {
        return filter_type::glow;
    } else if (type == "shadow") {
        return filter_type::shadow;
    }
    return filter_type::bypass;
}

std::string filter_type_to_string(filter_type type) {
    if (type == filter_type::refill) {
        return "refill";
    } else if (type == filter_type::glow) {
        return "glow";
    } else if (type == filter_type::shadow) {
        return "shadow";
    }
    return "bypass";
}

argb32_t parse_argb32(const std::string& str) {
    uint32_t c = 0x0;
    sscanf(str.c_str(), "#%08x", &c);
    return argb32_t{c};
}

void from_xml(const pugi::xml_node node, filter_data_t& filter) {
    filter.type = get_filter_type(node.attribute("type").as_string());
    filter.top = node.attribute("top").as_float(0.0f);
    filter.bottom = node.attribute("bottom").as_float(100.0f);
    filter.distance = node.attribute("distance").as_float(0.0f);
    filter.quality = node.attribute("quality").as_int(1);
    filter.strength = node.attribute("strength").as_float(1.0f);
    filter.angle = ek::math::to_radians(node.attribute("angle").as_float(0.0f));
    filter.blur.x = node.attribute("blur_x").as_float(4.0f);
    filter.blur.y = node.attribute("blur_y").as_float(4.0f);
    filter.color = parse_argb32(node.attribute("color").as_string("#FF000000"));
    filter.color_bottom = parse_argb32(node.attribute("color_bottom").as_string("#FF000000"));
}

void to_xml(pugi::xml_node node, const filter_data_t& filter) {
    node.append_attribute("type").set_value(filter_type_to_string(filter.type).c_str());
    node.append_attribute("top").set_value(filter.top);
    node.append_attribute("bottom").set_value(filter.bottom);
    node.append_attribute("distance").set_value(filter.distance);
    node.append_attribute("quality").set_value(filter.quality);
    node.append_attribute("strength").set_value(filter.strength);
    node.append_attribute("angle").set_value(ek::math::to_degrees(filter.angle));
    node.append_attribute("blur_x").set_value(filter.blur.x);
    node.append_attribute("blur_y").set_value(filter.blur.y);
    node.append_attribute("color").set_value(("#" + to_hex(filter.color.argb)).c_str());
    node.append_attribute("color_bottom").set_value(("#" + to_hex(filter.color_bottom.argb)).c_str());
}

void from_xml(const pugi::xml_node node, filters_decl_t& filters) {
    for (auto& filter_node: node.children("filter")) {
        filter_data_t f{};
        from_xml(filter_node, f);
        filters.filters.push_back(f);
    }
}

void to_xml(pugi::xml_node node, const filters_decl_t& filters) {
    for (auto& filter: filters.filters) {
        auto filter_node = node.append_child("filter");
        to_xml(filter_node, filter);
    }
}

}