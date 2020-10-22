#pragma once

#include <ek/util/common_macro.hpp>
#include <ek/math/box.hpp>
#include <ek/util/assets.hpp>

namespace ek {

namespace graphics {
class texture_t;
}

class sprite_t : private disable_copy_assign_t {
public:
    asset_t<graphics::texture_t> texture;
    rect_f rect{0, 0, 1, 1};
    rect_f tex{0, 0, 1, 1};
    bool rotated = false;

    sprite_t() = default;

    ~sprite_t() = default;

    void draw() const;

    void draw(const rect_f& rc) const;

    void draw_grid(const rect_f& grid, const rect_f& target) const;

    [[nodiscard]] bool hit_test(const float2& position) const;

    [[nodiscard]] bool select() const;
};

}

