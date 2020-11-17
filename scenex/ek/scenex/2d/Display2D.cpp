#include "Display2D.hpp"
#include <ek/draw2d/drawer.hpp>
#include <ek/scenex/2d/Sprite.hpp>
#include <ek/scenex/text/Font.hpp>
#include <ek/util/assets.hpp>
#include <ek/scenex/text/TextDrawer.hpp>
#include <ek/math/bounds_builder.hpp>

namespace ek {

IDrawable2D::~IDrawable2D() = default;

void Quad2D::draw() {
    draw2d::state.set_empty_texture();
    draw2d::quad(rect.x, rect.y, rect.width, rect.height, colors[0], colors[1], colors[2], colors[3]);
}

rect_f Quad2D::getBounds() const {
    return rect;
}

bool Quad2D::hitTest(float2 point) const {
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

Sprite2D::Sprite2D() = default;

Sprite2D::Sprite2D(const std::string& sprite_id, rect_f a_scale_grid)
        : src{sprite_id},
          scale_grid{a_scale_grid} {
    scale_grid_mode = !scale_grid.empty();
}

void Sprite2D::draw() {
    if (!src) {
        return;
    }
    auto& spr = *src;
    if (scale_grid_mode) {
//				scale.set(5, 5);
        draw2d::state
                .save_matrix()
                .scale(1.0f / scale.x, 1.0f / scale.y);
        // TODO: rotated
        rect_f target{manual_target};
        if (target.empty()) {
            target = {(spr.rect.position + float2{1.0f, 1.0f}) * scale,
                      (spr.rect.size - float2{2.0f, 2.0f}) * scale};
        }
        spr.draw_grid(scale_grid, target);
//            drawer.set_empty_texture();
//            drawer.quad(target.x, target.y, target.width, target.height, 0x77FF0000_argb);
//            drawer.quad(scale_grid.x, scale_grid.y, scale_grid.width, scale_grid.height, 0x77FFFFFF_argb);
        draw2d::state.restore_matrix();
    } else {
//            drawer.set_empty_texture();
//            drawer.quad(spr->rect, 0x77FFFFFF_argb);
        spr.draw();
    }
}

rect_f Sprite2D::getBounds() const {
    if (!src) {
        return rect_f::zero;
    }
    auto& spr = *src;
    return spr.rect;
}

bool Sprite2D::hitTest(float2 point) const {
    if (getBounds().contains(point)) {
        if (hit_pixels && src) {
            return src->hit_test(point);
        }
        return true;
    }
    return false;
}

Text2D::Text2D()
        : text{},
          format{"mini", 16.0f} {

}

Text2D::Text2D(std::string text, TextFormat format)
        : text{std::move(text)},
          format{format} {
}

void Text2D::draw() {
    auto& textDrawer = TextDrawer::shared;
    auto& blockInfo = TextDrawer::sharedTextBlockInfo;
    textDrawer.format = format;
    if (fillColor.a > 0) {
        draw2d::state.set_empty_texture();
        draw2d::quad(rect, fillColor);
    }
    if (borderColor.a > 0) {
        draw2d::state.set_empty_texture();
        draw2d::strokeRect(expand(rect, 1.0f), borderColor, 1);
    }
    textDrawer.getTextSize(text.c_str(), blockInfo);

    const float2 position = rect.position + (rect.size - blockInfo.size) * format.alignment;
    textDrawer.position = position + float2{0.0f, blockInfo.line[0].y};
    textDrawer.drawWithBlockInfo(text.c_str(), blockInfo);
    if (showTextBounds) {
        const rect_f bounds{position, blockInfo.size};
        draw2d::strokeRect(expand(bounds, 1.0f), 0xFF0000_rgb, 1);
    }
}

rect_f Text2D::getBounds() const {
    if (hitFullBounds) {
        return rect;
    }
    auto& textDrawer = TextDrawer::shared;
    auto& blockInfo = TextDrawer::sharedTextBlockInfo;
    textDrawer.format = format;
    textDrawer.getTextSize(text.c_str(), blockInfo);
    return {
            rect.position + (rect.size - blockInfo.size) * format.alignment,
            blockInfo.size
    };
}

bool Text2D::hitTest(float2 point) const {
    return getBounds().contains(point);
}

rect_f scissors_2d::world_rect(const matrix_2d& world_matrix) const {
    bounds_builder_2f bb{};
    bb.add(rect, world_matrix);
    return bb.rect();
}

//// arc

void Arc2D::draw() {
    Res<Sprite> f{sprite};
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

rect_f Arc2D::getBounds() const {
    float s = radius + line_width;
    return {-s, -s, 2.0f * s, 2.0f * s};
}

bool Arc2D::hitTest(float2 point) const {
    auto len = length(point);
    return len >= radius && len <= (radius + line_width);
}

/** utilities **/
void set_gradient_quad(ecs::entity e, const rect_f& rc, argb32_t top, argb32_t bottom) {
    auto q = std::make_unique<Quad2D>();
    q->rect = rc;
    q->colors[0] = top;
    q->colors[1] = top;
    q->colors[2] = bottom;
    q->colors[3] = bottom;

    e.get_or_create<Display2D>().drawable = std::move(q);
}
}