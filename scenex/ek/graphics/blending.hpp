#pragma once

#include "gl_def.hpp"

namespace ek::graphics {

enum class blend_factor : GLuint {
    zero = GL_ZERO,
    one = GL_ONE,
    one_minus_src_alpha = GL_ONE_MINUS_SRC_ALPHA,
    one_minus_src_color = GL_ONE_MINUS_SRC_COLOR,
    one_minus_dest_color = GL_ONE_MINUS_DST_COLOR
};

struct blend_mode {
    blend_factor source;
    blend_factor dest;

    bool operator==(const blend_mode& a) const {
        return (source == a.source && dest == a.dest);
    }

    bool operator!=(const blend_mode& a) const {
        return !operator==(a);
    }

    const static blend_mode nop;
    const static blend_mode opaque;
    const static blend_mode premultiplied;
    const static blend_mode add;
    const static blend_mode screen;
    const static blend_mode exclusion;
};

inline const blend_mode blend_mode::nop{blend_factor::zero, blend_factor::zero};
inline const blend_mode blend_mode::opaque{blend_factor::one, blend_factor::zero};
inline const blend_mode blend_mode::premultiplied{blend_factor::one, blend_factor::one_minus_src_alpha};
inline const blend_mode blend_mode::add{blend_factor::one, blend_factor::one_minus_src_alpha};
inline const blend_mode blend_mode::screen{blend_factor::one, blend_factor::one_minus_src_color};
inline const blend_mode blend_mode::exclusion{blend_factor::one_minus_dest_color, blend_factor::one_minus_src_color};

}