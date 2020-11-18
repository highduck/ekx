#include "ParticleDecl.hpp"

namespace ek {

argb32_t rand_color_data::next() const {
    ++state;
    int index_max = int(colors.size() - 1);
    if (colors.empty()) {
        return argb32_t{0xFFFFFFFF};
    }
    if (colors.size() == 1) return colors[0];
    switch (mode) {
        case rand_color_mode::RandLerp: {
            float t = rand_fx.random(0.0f, float(index_max));
            int i = int(t);
            t = math::fract(t);
            return lerp(colors[i], colors[i + 1], t);
        }
        case rand_color_mode::RandElement:
            return colors[rand_fx.random_int(0, index_max)];
        case rand_color_mode::Continuous:
            return colors[state % colors.size()];
    }
    return argb32_t{0xFFFFFFFF};
}
}