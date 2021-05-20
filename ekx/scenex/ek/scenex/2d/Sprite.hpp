#pragma once

#include <ek/util/NoCopyAssign.hpp>
#include <ek/math/box.hpp>
#include <ek/util/Res.hpp>

namespace ek {

namespace graphics {
class Texture;
}

class Sprite : private NoCopyAssign {
public:
    Res<graphics::Texture> texture;
    rect_f rect{0, 0, 1, 1};
    rect_f tex{0, 0, 1, 1};
    bool rotated = false;

    Sprite() = default;

    ~Sprite() = default;

    void draw() const;

    void draw(const rect_f& rc) const;

    void draw_grid(const rect_f& grid, const rect_f& target) const;

    [[nodiscard]] bool hit_test(const float2& position) const;

    [[nodiscard]] bool select() const;
};

}

