#include "parsing.hpp"

#include <ek/flash/doc/types.hpp>
#include <ek/util/logger.hpp>
#include <pugixml.hpp>

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
    if (equals(str, "BitmapFill")) return r = fill_type::bitmap;
    return r = fill_type::unknown;
}

spread_method& operator<<(spread_method& r, const char* str) {
    //const char* str = node.attribute("spreadMethod").value();
    if (equals(str, "reflect")) return r = spread_method::reflect;
    if (equals(str, "repeat")) return r = spread_method::repeat;
    return r = spread_method::extend;
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
            case fill_type::bitmap:
                r.spreadMethod = spread_method::repeat;
                r.matrix << el;
                r.matrix = r.matrix.scale(1.0f / 20.0f, 1.0f / 20.0f);
                r.bitmapPath = el.attribute("bitmapPath").value();
                // NOTE: bitmap ref will be resolved in dom_scanner
                if (!r.bitmap) {
                    EK_WARN << "[BitmapFill] bitmap item not found: " << r.bitmapPath;
                }
                break;
            default:
                break;
        }
        if(math::equals(det(r.matrix), 0.0f)) {
            r.type = fill_type::solid;
        }
    }
    return r;
}

stroke_style& operator<<(stroke_style& r, const xml_node& node) {
    r.index = node.attribute("index").as_int();

    auto solid = node.child("SolidStroke");

    if(!solid.empty()) {
        r.is_solid = true;
        r.weight = solid.attribute("weight").as_float(1.0f);
        r.scaleMode << solid.attribute("scaleMode").value();
        r.fill << solid.child("fill");
        r.miterLimit = solid.attribute("miterLimit").as_float(3.0f);
        r.pixelHinting = solid.attribute("miterLimit").as_bool(false);
        r.caps << solid.attribute("caps").value();
        r.joints << solid.attribute("joints").value();
        r.solidStyle << solid.attribute("solidStyle").value();
    }
    else {
        r.is_solid = false;
    }

    return r;
}

}