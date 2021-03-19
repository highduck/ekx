#pragma once

#include "graphics.hpp"
#include <array>
#include <functional>

namespace ek {
class image_t;
}

namespace ek::graphics {

//struct blend_mode {
//    blend_factor source;
//    blend_factor dest;
//
//    bool operator==(const blend_mode& a) const {
//        return (source == a.source && dest == a.dest);
//    }
//
//    bool operator!=(const blend_mode& a) const {
//        return !operator==(a);
//    }
//
//    const static blend_mode nop;
//    const static blend_mode opaque;
//    const static blend_mode premultiplied;
//    const static blend_mode add;
//    const static blend_mode screen;
//    const static blend_mode exclusion;
//};

//extern const sg_blend_state BlendModeDisabled;
//inline const blend_mode blend_mode::opaque{blend_factor::one, blend_factor::zero};
//inline const blend_mode blend_mode::premultiplied{blend_factor::one, blend_factor::one_minus_src_alpha};
//inline const blend_mode blend_mode::add{blend_factor::one, blend_factor::one_minus_src_alpha};
//inline const blend_mode blend_mode::screen{blend_factor::one, blend_factor::one_minus_src_color};
//inline const blend_mode blend_mode::exclusion{blend_factor::one_minus_dest_color, blend_factor::one_minus_src_color};

Texture* createTexture(const image_t& image);
Texture* createTexture(std::array<ek::image_t*, 6> images);
Texture* createRenderTarget(int width, int height);
void load_texture_lazy(const char* path, std::function<void(Texture*)> callback);
//void load_texture_cube_lazy(const std::vector<std::string>& path_list, std::function<void(Texture*)> callback);

}