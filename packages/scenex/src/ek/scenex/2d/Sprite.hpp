#pragma once

#include <ek/util/NoCopyAssign.hpp>
#include <ek/math/Rect.hpp>
#include <ek/util/Type.hpp>
#include <ek/util/Res.hpp>
#include <ek/graphics/graphics.hpp>

namespace ek {

class Sprite : private NoCopyAssign {
public:
    ek_image_reg_id image_id = {0};
    Rect2f rect{0, 0, 1, 1};
    Rect2f tex{0, 0, 1, 1};
    bool rotated = false;

    Sprite() = default;

    ~Sprite() = default;

    void draw() const;

    void draw(const Rect2f& rc) const;

    void draw_grid(const Rect2f& grid, const Rect2f& target) const;

    [[nodiscard]] bool hit_test(const Vec2f& position) const;

    [[nodiscard]] bool select() const;
};

EK_DECLARE_TYPE(Sprite);
EK_TYPE_INDEX(Sprite, 4);

}

