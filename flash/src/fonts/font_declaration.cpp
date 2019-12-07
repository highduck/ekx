#include <ek/util/strings.hpp>
#include "font_declaration.hpp"

namespace ek {

void from_xml(const pugi::xml_node node, font_decl_t& font) {
    for (auto& size_node: node.children("size")) {
        font.sizes.push_back(size_node.attribute("value").as_int(16));
    }

    for (auto& range_node: node.children("code_range")) {
        ek::font_lib::code_range_t cr{0x0020u, 0x007Fu};
        cr.min = std::strtoul(range_node.attribute("from").as_string("0x0020"), nullptr, 16);
        cr.max = std::strtoul(range_node.attribute("to").as_string("0x007F"), nullptr, 16);
        font.ranges.push_back(cr);
    }

    if (font.ranges.empty()) {
        font.ranges.push_back({0x0020u, 0x007Fu});
    }

    font.mirror_case = node.attribute("mirror_case").as_bool(false);
    font.use_kerning = node.attribute("use_kerning").as_bool(true);
}

void to_xml(pugi::xml_node node, const font_decl_t& font) {
    for (uint16_t size: font.sizes) {
        node.append_child("size").append_attribute("value").set_value(size);
    }

    for (const auto& range: font.ranges) {
        auto range_node = node.append_child("code_range");
        range_node.append_attribute("from").set_value(("0x" + to_hex(range.min)).c_str());
        range_node.append_attribute("to").set_value(("0x" + to_hex(range.max)).c_str());
    }

    node.append_attribute("mirror_case").set_value(font.mirror_case);
    node.append_attribute("use_kerning").set_value(font.use_kerning);
}

}