#include "Sprite.hpp"

#include <ek/canvas.h>

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
    const sg_image image = ek_ref_content(sg_image, image_id);
    if (image.id) {
        canvas_set_image_region(image, tex);
        return true;
    }
    return false;
}

void Sprite::draw() const {
    draw(rect);
}

void Sprite::draw(const rect_t rc) const {
    const sg_image image = ek_ref_content(sg_image, image_id);
    if (image.id) {
        canvas_set_image(image);
        canvas_set_image_rect(tex);
        if (rotated) {
            canvas_quad_rotated(rc.x, rc.y, rc.w, rc.h);
        } else {
            canvas_quad(rc.x, rc.y, rc.w, rc.h);
        }
    }
}

void Sprite::draw_grid(const rect_t grid, const rect_t target) const {
    const sg_image image = ek_ref_content(sg_image, image_id);
    if (image.id == SG_INVALID_ID) {
        return;
    }

    canvas_set_image_region(image, tex);

    float x = rect.x;
    float y = rect.y;
    float width = rect.w;
    float height = rect.h;
    float tx = target.x;
    float ty = target.y;
    float tw = target.w;
    float th = target.h;

    float x0 = tx;
    float x1 = tx + grid.x - x;
    float x2 = tx + tw - ((x + width) - RECT_R(grid));
    float x3 = tx + tw;

    float y0 = ty;
    float y1 = ty + grid.y - y;
    float y2 = ty + th - ((y + height) - RECT_B(grid));
    float y3 = ty + th;

    float u0 = 0;
    float u1 = (grid.x - x) / width;
    float u2 = 1 - (((x + width) - RECT_R(grid)) / width);
    float u3 = 1;
    float v0 = 0;
    float v1 = (grid.y - y) / height;
    float v2 = 1 - (((y + height) - RECT_B(grid)) / height);
    float v3 = 1;

    canvas_triangles(4 * 4, 6 * 9);
    auto cm = canvas.color[0].scale;
    auto co = canvas.color[0].offset;
    /////
    canvas_write_vertex(x0, y0, u0, v0, cm, co);
    canvas_write_vertex(x1, y0, u1, v0, cm, co);
    canvas_write_vertex(x2, y0, u2, v0, cm, co);
    canvas_write_vertex(x3, y0, u3, v0, cm, co);

    canvas_write_vertex(x0, y1, u0, v1, cm, co);
    canvas_write_vertex(x1, y1, u1, v1, cm, co);
    canvas_write_vertex(x2, y1, u2, v1, cm, co);
    canvas_write_vertex(x3, y1, u3, v1, cm, co);

    canvas_write_vertex(x0, y2, u0, v2, cm, co);
    canvas_write_vertex(x1, y2, u1, v2, cm, co);
    canvas_write_vertex(x2, y2, u2, v2, cm, co);
    canvas_write_vertex(x3, y2, u3, v2, cm, co);

    canvas_write_vertex(x0, y3, u0, v3, cm, co);
    canvas_write_vertex(x1, y3, u1, v3, cm, co);
    canvas_write_vertex(x2, y3, u2, v3, cm, co);
    canvas_write_vertex(x3, y3, u3, v3, cm, co);

    canvas_write_indices(nine_patch_indices, 9 * 6, 0);
}

bool Sprite::hit_test(const vec2_t position) const {
    (void) position;
    const sg_image image = ek_ref_content(sg_image, image_id);
    if (image.id) {
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