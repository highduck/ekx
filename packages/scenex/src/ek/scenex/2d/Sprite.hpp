#pragma once

#include <ek/util/NoCopyAssign.hpp>
#include <ek/util/Type.hpp>
#include <ek/gfx.h>
#include <ek/math.h>

namespace ek {

class Sprite : private NoCopyAssign {
public:
    rect_t rect = rect_01();
    rect_t tex = rect_01();
    ek_ref(sg_image) image_id = {0};
    bool rotated = false;

    Sprite() = default;

    ~Sprite() = default;

    void draw() const;

    void draw(rect_t rc) const;

    void draw_grid(rect_t grid, rect_t target) const;

    [[nodiscard]] bool hit_test(vec2_t position) const;

    [[nodiscard]] bool select() const;
};

EK_DECLARE_TYPE(Sprite);
EK_TYPE_INDEX(Sprite, 4);

}

