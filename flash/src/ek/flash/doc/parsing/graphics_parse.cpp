#include "graphics_parse.h"

#include "basic_types.h"
#include <ek/flash/doc/graphic_types.h>

#include <pugixml.hpp>
#include <ek/util/logger.hpp>

namespace ek::flash {

scale_mode& operator<<(scale_mode& r, const char* str) {
    if (equals(str, "normal")) return r = scale_mode::normal;
    else if (equals(str, "horizontal")) return r = scale_mode::horizontal;
    else if (equals(str, "vertical")) return r = scale_mode::vertical;
    else if (equals(str, "none")) return r = scale_mode::none;
    else if (str && *str) EK_WARN("unknown ScaleMode: %s", str);
    return r = scale_mode::none;
}

solid_style_type& operator<<(solid_style_type& r, const char* str) {
    if (equals(str, "hairline")) return r = solid_style_type::hairline;
    else if (str && *str) EK_WARN("unknown SolidStyle: %s", str);
    return r = solid_style_type::hairline;
}

line_caps& operator<<(line_caps& r, const char* str) {
    if (equals(str, "none")) return r = line_caps::none;
    else if (equals(str, "round")) return r = line_caps::round;
    else if (equals(str, "square")) return r = line_caps::square;
    else if (str && *str) EK_WARN("unknown LineCaps: %s", str);
    return r = line_caps::round;
}

line_joints& operator<<(line_joints& r, const char* str) {
    if (equals(str, "miter")) return r = line_joints::miter;
    else if (equals(str, "round")) return r = line_joints::round;
    else if (equals(str, "bevel")) return r = line_joints::bevel;
    else if (str && *str) EK_WARN("unknown LineJoints: %s", str);
    return r = line_joints::round;
}

fill_type& operator<<(fill_type& r, const char* str) {
    if (equals(str, "SolidColor")) return r = fill_type::solid;
    if (equals(str, "LinearGradient")) return r = fill_type::linear;
    if (equals(str, "RadialGradient")) return r = fill_type::radial;
    return r = fill_type::unknown;
}

spread_method& operator<<(spread_method& r, const char* str) {
    //const char* str = node.attribute("spreadMethod").value();
    if (equals(str, "reflect")) return r = spread_method::reflect;
    if (equals(str, "repeat")) return r = spread_method::repeat;
    return r = spread_method::extend;
}


solid_stroke& operator<<(solid_stroke& solid, const xml_node& node) {
    solid.weight = node.attribute("weight").as_float(1.0f);
    solid.scaleMode << node.attribute("scaleMode").value();
    solid.fill = read_color(node.child("fill").child("SolidColor"));
    solid.miterLimit = node.attribute("miterLimit").as_float(3.0f);
    solid.pixelHinting = node.attribute("miterLimit").as_bool(false);
    solid.caps << node.attribute("caps").value();
    solid.joints << node.attribute("joints").value();
    solid.solidStyle << node.attribute("solidStyle").value();
    return solid;
}

gradient_entry& operator<<(gradient_entry& r, const xml_node& node) {
    r.color = read_color(node);
    r.ratio = node.attribute("ratio").as_float();
    return r;
}

fill_style& operator<<(fill_style& r, const xml_node& node) {
    r.index = node.attribute("index").as_int();
    for (const auto& el : node.children()) {
        r.type << el.name();
        switch (r.type) {
            case fill_type::solid:
                r.entries.push_back(parse_xml_node<gradient_entry>(el));
                break;
            case fill_type::linear:
            case fill_type::radial:
                r.spreadMethod << el.attribute("spreadMethod").value();
                r.matrix << el;
                for (const auto& e: el.children("GradientEntry")) {
                    r.entries.push_back(parse_xml_node<gradient_entry>(e));
                }
                break;
            default:
                break;
        }
    }
    return r;
}

stroke_style& operator<<(stroke_style& r, const xml_node& node) {
    r.index = node.attribute("index").as_int();
    auto el = node.child("SolidStroke");
    r.solid << el;
    r.is_solid = !el.empty();
    return r;
}
}