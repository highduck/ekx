#include "parse_blend_mode.h"

#include "basic_types.h"
#include <ek/flash/doc/blend_mode.h>
#include <ek/util/logger.hpp>

namespace ek::flash {

blend_mode_t& operator<<(blend_mode_t& r, const char* str) {
    if (!str || !(*str)) return r = blend_mode_t::last;
    else if (equals(str, "normal")) return r = blend_mode_t::normal;
    else if (equals(str, "layer")) return r = blend_mode_t::layer;
    else if (equals(str, "multiply")) return r = blend_mode_t::multiply;
    else if (equals(str, "screen")) return r = blend_mode_t::screen;
    else if (equals(str, "lighten")) return r = blend_mode_t::lighten;
    else if (equals(str, "darken")) return r = blend_mode_t::darken;
    else if (equals(str, "difference")) return r = blend_mode_t::difference;
    else if (equals(str, "add")) return r = blend_mode_t::add;
    else if (equals(str, "subtract")) return r = blend_mode_t::subtract;
    else if (equals(str, "invert")) return r = blend_mode_t::invert;
    else if (equals(str, "alpha")) return r = blend_mode_t::alpha;
    else if (equals(str, "erase")) return r = blend_mode_t::erase;
    else if (equals(str, "overlay")) return r = blend_mode_t::overlay;
    else if (equals(str, "hardlight")) return r = blend_mode_t::hardlight;
    EK_WARN("unknown BlendMode: %s", str);
    return r = blend_mode_t::normal;
}

}