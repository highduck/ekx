#include "display_2d.hpp"
#include <ek/draw2d/drawer.hpp>
#include <ek/scenex/2d/sprite.hpp>
#include <ek/scenex/text/font.hpp>
#include <ek/util/assets.hpp>
#include <ek/scenex/text/text_drawer.hpp>

namespace ek {

drawable_2d_base::~drawable_2d_base() = default;

void drawable_quad::draw() {
    draw2d::state.set_empty_texture();
    draw2d::quad(rect.x, rect.y, rect.width, rect.height, colors[0], colors[1], colors[2], colors[3]);
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

drawable_sprite::drawable_sprite() = default;

drawable_sprite::drawable_sprite(std::string sprite_id, rect_f a_scale_grid)
        : src{std::move(sprite_id)},
          scale_grid{a_scale_grid} {
    scale_grid_mode = !scale_grid.empty();
}

void drawable_sprite::draw() {
    asset_t<sprite_t> spr{src};
    if (spr) {
        if (scale_grid_mode) {
//				scale.set(5, 5);
            draw2d::state
                    .save_matrix()
                    .scale(1.0f / scale.x, 1.0f / scale.y);
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
            draw2d::state.restore_matrix();
        } else {
//            drawer.set_empty_texture();
//            drawer.quad(spr->rect, 0x77FFFFFF_argb);
            spr->draw();
        }
    }
}

rect_f drawable_sprite::get_bounds() const {
    asset_t<sprite_t> spr{src};
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
            asset_t<sprite_t> spr{src};
            if (spr) {
                return spr->hit_test(point);
            }
        }
        return true;
    }
    return false;
}

drawable_text::drawable_text()
        : text{},
          format{"mini", 16.0f} {

}

drawable_text::drawable_text(std::string text, TextFormat format)
        : text{std::move(text)},
          format{format} {
}

void drawable_text::draw() {
    TextDrawer drawer;
    drawer.format = format;
//    drawer.rect = rect;
//    drawer.alignment = format.alignment;
    if (fillColor.a > 0) {
        draw2d::state.set_empty_texture();
        draw2d::quad(rect, fillColor);
    }
    if (borderColor.a > 0) {
        draw2d::state.set_empty_texture();
        draw2d::strokeRect(expand(rect, 1.0f), borderColor, 1);
    }
    auto& info = TextDrawer::sharedTextBlockInfo;
    drawer.getTextSize(text.c_str(), info);
    auto bounds = info.bounds;
    drawer.rect.position = rect.relative(format.alignment) - bounds.relative(format.alignment);
    bounds.position += drawer.rect.position;
    drawer.rect.size = {};
    drawer.draw(text.c_str());
    if (showTextBounds) {
        draw2d::strokeRect(expand(bounds, 1.0f), 0xFF0000_rgb, 1);
    }
}

rect_f drawable_text::get_bounds() const {
    if (hitFullBounds) {
        return rect;
    }
    TextDrawer drawer;
    drawer.format = format;
    auto& info = TextDrawer::sharedTextBlockInfo;
    drawer.getTextSize(text.c_str(), info);
    auto bounds = info.bounds;
    bounds.position += rect.relative(format.alignment) - bounds.relative(format.alignment);
    return bounds;
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
    asset_t<sprite_t> f{sprite};
    if (f && f->texture) {
        auto& tex = f->tex;
        draw2d::state.set_texture_region(
                f->texture.get(),
                {tex.center_x(), tex.y, 0.0f, tex.height}
        );

        float off = -math::pi * 0.5f;
        draw2d::line_arc(0, 0, radius, 0 + off, angle + off, line_width, segments, color_inner, color_outer);
    }
}

rect_f drawable_arc::get_bounds() const {
    float s = radius + line_width;
    return {-s, -s, 2.0f * s, 2.0f * s};
}

bool drawable_arc::hit_test(const float2& point) const {
    auto len = length(point);
    return len >= radius && len <= (radius + line_width);
}
}