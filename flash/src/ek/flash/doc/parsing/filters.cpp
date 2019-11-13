#include "filters.h"

#include "basic_types.h"
#include <ek/flash/doc/filter.h>

#include <pugixml.hpp>

namespace ek::flash {

filter_kind_t& operator<<(filter_kind_t& r, const char* str) {
    if (equals(str, "DropShadowFilter")) {
        return r = filter_kind_t::drop_shadow;
    } else if (equals(str, "GlowFilter")) {
        return r = filter_kind_t::glow;
    } else if (equals(str, "BevelFilter")) {
        return r = filter_kind_t::bevel;
    } else if (equals(str, "BlurFilter")) {
        return r = filter_kind_t::blur;
    } else if (equals(str, "ConvolutionFilter")) {
        return r = filter_kind_t::convolution;
    } else if (equals(str, "AdjustColorFilter")) {
        return r = filter_kind_t::adjust_color;
    } else if (equals(str, "GradientBevelFilter")) {
        return r = filter_kind_t::bevel;
    } else if (equals(str, "GradientGlowFilter")) {
        return r = filter_kind_t::glow;
    }
    return r = filter_kind_t::none;
}

filter_t& operator<<(filter_t& r, const xml_node& node) {
    r.type << node.name();
    r.color = read_color(node);
    r.blur.x = node.attribute("blurX").as_float(4.0f);
    r.blur.y = node.attribute("blurY").as_float(4.0f);
    r.distance = node.attribute("distance").as_float(4.0f);
    r.angle = node.attribute("angle").as_float(45.0f);
    r.quality = node.attribute("quality").as_int(1);
    r.strength = node.attribute("strength").as_float(100.0f);
    r.inner = node.attribute("inner").as_bool(false);
    r.knockout = node.attribute("knockout").as_bool(false);
    r.hideObject = node.attribute("hideObject").as_bool(false);
    return r;
}

}
