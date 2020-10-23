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

}