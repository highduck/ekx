#include <ek/util/strings.hpp>
#include "font_declaration.hpp"

namespace ek {

void from_xml(const pugi::xml_node node, font_decl_t& font) {
    font.fontSize = node.attribute("fontSize").as_int(16);

    for (auto& range_node: node.children("code_range")) {
        ek::font_lib::code_range_t cr{0x0020u, 0x007Fu};
        cr.min = std::strtoul(range_node.attribute("from").as_string("0x0020"), nullptr, 16);
        cr.max = std::strtoul(range_node.attribute("to").as_string("0x007F"), nullptr, 16);
        font.ranges.push_back(cr);
    }

    if (font.ranges.empty()) {
        font.ranges.push_back({0x0020u, 0x007Fu});
    }

    font.mirrorCase = node.attribute("mirror_case").as_bool(false);
    font.useKerning = node.attribute("use_kerning").as_bool(true);
}

void to_xml(pugi::xml_node node, const font_decl_t& font) {
    node.append_attribute("fontSize").set_value(font.fontSize);

    for (const auto& range: font.ranges) {
        auto range_node = node.append_child("code_range");
        range_node.append_attribute("from").set_value(("0x" + to_hex(range.min)).c_str());
        range_node.append_attribute("to").set_value(("0x" + to_hex(range.max)).c_str());
    }

    node.append_attribute("mirror_case").set_value(font.mirrorCase);
    node.append_attribute("use_kerning").set_value(font.useKerning);
}

}