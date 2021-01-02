#include "Sprite.hpp"

#include <ek/draw2d/drawer.hpp>

namespace ek {

/**** draw routines ****/

static unsigned short nine_patch_indices[] = {
        0, 1, 5, 5, 4, 0,
        1, 1 + 1, 1 + 5, 1 + 5, 1 + 4, 1,
        2, 2 + 1, 2 + 5, 2 + 5, 2 + 4, 2,
        4, 4 + 1, 4 + 5, 4 + 5, 4 + 4, 4,
        5, 5 + 1, 5 + 5, 5 + 5, 5 + 4, 5,
        6, 6 + 1, 6 + 5, 6 + 5, 6 + 4, 6,
        8, 8 + 1, 8 + 5, 8 + 5, 8 + 4, 8,
        9, 9 + 1, 9 + 5, 9 + 5, 9 + 4, 9,
        10, 10 + 1, 10 + 5, 10 + 5, 10 + 4, 10
};

bool Sprite::select() const {
    if (texture) {
        draw2d::current().set_texture_region(texture.get(), tex);
        return true;
    }
    return false;
}

void Sprite::draw() const {
    draw(rect);
}

void Sprite::draw(const rect_f& rc) const {
    if (texture) {
        draw2d::current().set_texture_region(texture.get(), tex);
        if (rotated) {
            draw2d::quad_rotated(rc.x, rc.y, rc.width, rc.height);
        } else {
            draw2d::quad(rc.x, rc.y, rc.width, rc.height);
        }
    }
}

void Sprite::draw_grid(const rect_f& grid, const rect_f& target) const {
    if (!texture) {
        return;
    }

    draw2d::current().set_texture_region(texture.get(), tex);

    float x = rect.x;
    float y = rect.y;
    float width = rect.width;
    float height = rect.height;
    float tx = target.x;
    float ty = target.y;
    float tw = target.width;
    float th = target.height;

    float x0 = tx;
    float x1 = tx + grid.x - x;
    float x2 = tx + tw - ((x + width) - grid.right());
    float x3 = tx + tw;

    float y0 = ty;
    float y1 = ty + grid.y - y;
    float y2 = ty + th - ((y + height) - grid.bottom());
    float y3 = ty + th;

    float u0 = 0;
    float u1 = (grid.x - x) / width;
    float u2 = 1 - (((x + width) - grid.right()) / width);
    float u3 = 1;
    float v0 = 0;
    float v1 = (grid.y - y) / height;
    float v2 = 1 - (((y + height) - grid.bottom()) / height);
    float v3 = 1;

    draw2d::triangles(4 * 4, 6 * 9);
    auto cm = draw2d::current().color.scale;
    auto co = draw2d::current().color.offset;
    /////
    draw2d::write_vertex(x0, y0, u0, v0, cm, co);
    draw2d::write_vertex(x1, y0, u1, v0, cm, co);
    draw2d::write_vertex(x2, y0, u2, v0, cm, co);
    draw2d::write_vertex(x3, y0, u3, v0, cm, co);

    draw2d::write_vertex(x0, y1, u0, v1, cm, co);
    draw2d::write_vertex(x1, y1, u1, v1, cm, co);
    draw2d::write_vertex(x2, y1, u2, v1, cm, co);
    draw2d::write_vertex(x3, y1, u3, v1, cm, co);

    draw2d::write_vertex(x0, y2, u0, v2, cm, co);
    draw2d::write_vertex(x1, y2, u1, v2, cm, co);
    draw2d::write_vertex(x2, y2, u2, v2, cm, co);
    draw2d::write_vertex(x3, y2, u3, v2, cm, co);

    draw2d::write_vertex(x0, y3, u0, v3, cm, co);
    draw2d::write_vertex(x1, y3, u1, v3, cm, co);
    draw2d::write_vertex(x2, y3, u2, v3, cm, co);
    draw2d::write_vertex(x3, y3, u3, v3, cm, co);

    draw2d::write_indices(nine_patch_indices, 9 * 6);
}

bool Sprite::hit_test(const float2& position) const {
    (void) position;
    if (texture) {
        return true;
    }
    return false;
//        if (texture) {
//            if (rotated) {
    //drawer.quadUVRotated(x, y, width, height);
//                return true;
//            } else {
//                drawer.quad(x, y, width, height);
//                return true;
//            }
//            return true;
//        }
//        return false;
}
}