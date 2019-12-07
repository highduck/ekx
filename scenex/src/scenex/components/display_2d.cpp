#include "display_2d.h"
#include <ek/util/locator.hpp>
#include <draw2d/drawer.hpp>
#include <scenex/2d/sprite.hpp>
#include <scenex/2d/font.hpp>
#include <ek/util/assets.hpp>

using namespace ek;

namespace scenex {

drawable_2d::~drawable_2d() = default;

void drawable_quad::draw() {
    auto& drawer = resolve<ek::drawer_t>();
    drawer.set_empty_texture();
    drawer.quad(rect.x, rect.y, rect.width, rect.height, colors[0], colors[1], colors[2], colors[3]);
}

rect_f drawable_quad::get_bounds() const {
    return rect;
}

bool drawable_quad::hit_test(const float2& point) const {
    return rect.contains(point);
}

//drawable_quad* drawable_quad::vGradient(const rect_f& rc, argb32_t top, argb32_t bottom) {
//    auto* q = new CQuad;
//    q->rect = rc;
//    q->colors[0] = top;
//    q->colors[1] = top;
//    q->colors[2] = bottom;
//    q->colors[3] = bottom;
//    return q;
//}
//
//drawable_quad* drawable_quad::colored(const rect_f& rc, argb32_t color) {
//    return vGradient(rc, color, color);
//}

drawable_sprite::drawable_sprite()
        : drawable_2d{type_id} {

}

drawable_sprite::drawable_sprite(std::string sprite_id, rect_f a_scale_grid)
        : drawable_2d{type_id},
          src{std::move(sprite_id)},
          scale_grid{a_scale_grid} {
    scale_grid_mode = !scale_grid.empty();
}

void drawable_sprite::draw() {
    using namespace ek;

    auto& drawer = resolve<drawer_t>();
    ek::asset_t<sprite_t> spr{src};
    if (spr) {
        if (scale_grid_mode) {
//				scale.set(5, 5);
            drawer.save_matrix();
            float sx = scale.x;
            float sy = scale.y;
            drawer.scale(1 / sx, 1 / sy);
            // TODO: rotated
            rect_f target{manual_target};
            if (target.empty()) {
                target = {(spr->rect.position + float2{1.0f, 1.0f}) * scale,
                          (spr->rect.size - float2{2.0f, 2.0f}) * scale};
            }
            spr->draw_grid(scale_grid, target);
//            drawer.set_empty_texture();
//            drawer.quad(target.x, target.y, target.width, target.height, 0x77FF0000_argb);
//            drawer.quad(scale_grid.x, scale_grid.y, scale_grid.width, scale_grid.height, 0x77FFFFFF_argb);
            drawer.restore_matrix();
        } else {
//            drawer.set_empty_texture();
//            drawer.quad(spr->rect, 0x77FFFFFF_argb);
            spr->draw();
        }
    }
}

rect_f drawable_sprite::get_bounds() const {
    ek::asset_t<sprite_t> spr{src};
    if (spr) {
//            if (scaleGridMode) {
//                float sx = scale.x;
//                float sy = scale.y;
//                return rect_f{spr->rect.position * scale, spr->rect.size * scale};
//            }
        return spr->rect;
    }
    return rect_f::zero;
}

bool drawable_sprite::hit_test(const float2& point) const {
    if (get_bounds().contains(point)) {
        if (hit_pixels) {
            ek::asset_t<sprite_t> spr{src};
            if (spr) {
                return spr->hit_test(point);
            }
        }
        return true;
    }
    return false;
}



//// text

static void draw_text(const std::string& text, const text_format_t& format, const rect_f& rc) {
    ek::asset_t<font_t> f{format.font};
    if (f) {

//        resolve<Drawer>().setEmptyTexture();
//        resolve<Drawer>().quad(rc.x, rc.y, rc.width, rc.height, 0x77FF0000_argb);
//        resolve<Drawer>().quad(rc.x + 2.0f, rc.y + 2.0f, rc.width - 4, rc.height - 4, 0x77000000_argb);

//        rect_f bounds = f->getLineBoundingBox(text, format.size, 0, text.size(), format.lineHeight, format.lineSpacing);
        const int begin = 0;
        const int end = static_cast<int>(text.size());
        rect_f draw_zone = f->estimate_text_draw_zone(
                text,
                format.size,
                begin,
                end,
                format.lineHeight,
                format.lineSpacing
        );
        float2 cur = rc.relative(format.alignment) - draw_zone.relative(format.alignment);

        if (format.shadow) {
            f->draw(text, format.size, cur + format.shadowOffset, format.shadowColor, format.lineHeight,
                    format.lineSpacing);
        }
        f->draw(text, format.size, cur, format.color, format.lineHeight, format.lineSpacing);
    }

}

static rect_f text_bounds(const std::string& text, const text_format_t& format, const rect_f& rc) {
    ek::asset_t<font_t> f(format.font);
    if (!f) {
        return rect_f::zero;
    }

    const int begin = 0;
    const int end = static_cast<int>(text.size());
    rect_f rect = f->get_line_bounding_box(text, format.size, begin, end, format.lineHeight);

    const float2 cur = rc.position + rc.size * format.alignment;
    rect.position += cur;

    float2 line_size{rect.width, format.size};
    rect.position.y += line_size.y;
    rect.position -= line_size * format.alignment;

    return rect;
}

drawable_text::drawable_text()
        : drawable_2d{type_id},
            text{},
          format{"mini", 16.0f} {

}

drawable_text::drawable_text(std::string text, text_format_t format)
        : drawable_2d{type_id},
        text{std::move(text)},
          format{std::move(format)} {
}

void drawable_text::draw() {
    draw_text(text, format, rect);
}

rect_f drawable_text::get_bounds() const {
    return text_bounds(text, format, rect);
}

bool drawable_text::hit_test(const float2& point) const {
    return get_bounds().contains(point);
}

rect_f scissors_2d::world_rect(const matrix_2d& world_matrix) const {
    return points_box(
            world_matrix.transform(rect.position),
            world_matrix.transform(rect.right_bottom())
    );
}

//// arc

void drawable_arc::draw() {
    ek::asset_t<sprite_t> f{sprite};
    if (f && f->texture) {
        auto& drawer = resolve<drawer_t>();
        drawer.set_texture(f->texture.get());

        auto& tex = f->tex;
        drawer.set_texture_coords(tex.x + tex.width * 0.5f, tex.y, 0.0f, tex.height);

        float off = -ek::math::pi * 0.5f;
        drawer.line_arc(0, 0, radius, 0 + off, angle + off, line_width, segments, color_inner, color_outer);
    }
}

rect_f drawable_arc::get_bounds() const {
    float s = radius + line_width;
    return {-s, -s, 2.0f * s, 2.0f * s};
}

bool drawable_arc::hit_test(const float2& point) const {
    auto len = ek::length(point);
    return len >= radius && len <= (radius + line_width);
}
}