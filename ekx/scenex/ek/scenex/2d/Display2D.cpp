#include "Display2D.hpp"
#include <ek/draw2d/drawer.hpp>
#include <ek/scenex/2d/Sprite.hpp>
#include <ek/scenex/text/Font.hpp>
#include <ek/util/Res.hpp>
#include <ek/scenex/text/TextEngine.hpp>
#include <ek/math/bounds_builder.hpp>
#include <ek/Localization.hpp>

namespace ek {

IDrawable2D::~IDrawable2D() = default;

void Quad2D::draw() {
    const Sprite* spr = src.get();
    if(spr) {
        const auto* texture = spr->texture.get();
        if(texture) {
            draw2d::state.setTextureRegion(texture, spr->tex);
        }
    }
    else {
        draw2d::state.setEmptyTexture();
    }
    draw2d::quad(rect.x, rect.y, rect.width, rect.height, colors[0], colors[1], colors[2], colors[3]);
}

rect_f Quad2D::getBounds() const {
    return rect;
}

bool Quad2D::hitTest(float2 point) const {
    return rect.contains(point);
}

/** Sprite2D **/
Sprite2D::Sprite2D() = default;

Sprite2D::Sprite2D(const std::string& spriteId) :
        src{spriteId} {
}

void Sprite2D::draw() {
    if (!src) {
        return;
    }
    auto& spr = *src;
    spr.draw();
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

/** NinePatch2D **/

NinePatch2D::NinePatch2D() = default;

NinePatch2D::NinePatch2D(const std::string& spriteId, rect_f aScaleGrid) :
        src{spriteId},
        scale_grid{aScaleGrid} {
}

void NinePatch2D::draw() {
    if (!src) {
        return;
    }
    auto& spr = *src;
    draw2d::state.save_matrix().scale(1.0f / scale.x, 1.0f / scale.y);
    // TODO: rotated
    rect_f target{manual_target};
    if (target.empty()) {
        target = {(spr.rect.position + float2{1.0f, 1.0f}) * scale,
                  (spr.rect.size - float2{2.0f, 2.0f}) * scale};
    }
    spr.draw_grid(scale_grid, target);
    draw2d::state.restore_matrix();
}

rect_f NinePatch2D::getBounds() const {
    if (!src) {
        return rect_f::zero;
    }
    auto& spr = *src;
    return spr.rect;
}

bool NinePatch2D::hitTest(float2 point) const {
    if (getBounds().contains(point)) {
        if (hit_pixels && src) {
            return src->hit_test(point);
        }
        return true;
    }
    return false;
}

/** Text2D **/

Text2D::Text2D()
        : text{},
          format{"mini", 16.0f} {

}

Text2D::Text2D(std::string text, TextFormat format)
        : text{std::move(text)},
          format{format} {
}

float findTextScale(float2 textSize, rect_f rect) {
    auto textScale = 1.0f;
    if (rect.width > 0.0f && textSize.x > 0.0f) {
        textScale = std::min(textScale, rect.width / textSize.x);
    }
    if (rect.height > 0.0f && textSize.y > 0.0f) {
        textScale = std::min(textScale, rect.height / textSize.y);
    }
    return textScale;
}

void adjustFontSize(TextEngine& engine, const char* text, rect_f bounds) {
    auto& info = gTextEngine.get().textBlockInfo;
    const float minFontSize = 10.0f;

    engine.format.allowLetterWrap = false;

    engine.getTextSize(text, info);

    if (engine.format.wordWrap) {
        while (engine.format.size > minFontSize &&
               (info.size.x > bounds.width || info.size.y > bounds.height)) {
            engine.format.size -= 1.0f;
            engine.getTextSize(text, info);
        }
    } else {
        const auto textScale = findTextScale(info.size, bounds);
        if (textScale < 1.0f) {
            engine.format.size *= textScale;
            if (engine.format.size < minFontSize) {
                engine.format.size = minFontSize;
            }
        }
    }
    engine.format.allowLetterWrap = true;
}

void Text2D::draw() {
    auto& textDrawer = gTextEngine.get().engine;
    auto& blockInfo = gTextEngine.get().textBlockInfo;
    textDrawer.format = format;
    textDrawer.maxWidth = format.wordWrap ? rect.width : 0.0f;
    if (fillColor.a > 0) {
        draw2d::state.setEmptyTexture();
        draw2d::quad(rect, fillColor);
    }
    if (borderColor.a > 0) {
        draw2d::state.setEmptyTexture();
        draw2d::strokeRect(expand(rect, 1.0f), borderColor, 1);
    }

    const char* str = localize ? Localization::instance.getText(text.c_str()) : text.c_str();
    if (str == nullptr || *str == '\0') {
        return;
    }

    if (adjustsFontSizeToFitBounds) {
        adjustFontSize(textDrawer, str, rect);
    }

    textDrawer.getTextSize(str, blockInfo);

    const float2 position = rect.position + (rect.size - blockInfo.size) * format.alignment;
    textDrawer.position = position + float2{0.0f, blockInfo.lines[0].ascender};
    textDrawer.drawWithBlockInfo(str, blockInfo);

    if (showTextBounds) {
        const rect_f bounds{position, blockInfo.size};
        draw2d::strokeRect(expand(bounds, 1.0f), 0xFF0000_rgb, 1);
    }
}

rect_f Text2D::getBounds() const {
    if (hitFullBounds) {
        return rect;
    }
    return getTextBounds();
}

rect_f Text2D::getTextBounds() const {
    auto& textDrawer = gTextEngine.get().engine;
    auto& blockInfo = gTextEngine.get().textBlockInfo;
    textDrawer.format = format;

    const char* str = localize ? Localization::instance.getText(text.c_str()) : text.c_str();

    if (adjustsFontSizeToFitBounds) {
        adjustFontSize(textDrawer, str, rect);
    }

    textDrawer.getTextSize(str, blockInfo);
    const auto textSize = blockInfo.size;

    rect_f textBounds{
            rect.position + (rect.size - textSize) * format.alignment,
            textSize
    };

    return textBounds;
}

bool Text2D::hitTest(float2 point) const {
    return getBounds().contains(point);
}

rect_f Bounds2D::getWorldRect(const matrix_2d& world_matrix) const {
    bounds_builder_2f bb{};
    bb.add(rect, world_matrix);
    return bb.rect();
}

rect_f Bounds2D::getScreenRect(matrix_2d viewMatrix, matrix_2d worldMatrix) const {
    auto lt = worldMatrix.transform(rect.position);
    auto rb = worldMatrix.transform(rect.right_bottom());

    lt = viewMatrix.transform(lt);
    rb = viewMatrix.transform(rb);

    return points_box(lt, rb);
}

//// arc

void Arc2D::draw() {
    Res<Sprite> f{sprite};
    if (f && f->texture) {
        auto& tex = f->tex;
        draw2d::state.setTextureRegion(
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
void set_gradient_quad(ecs::EntityApi e, const rect_f& rc, argb32_t top, argb32_t bottom) {
    auto q = std::make_unique<Quad2D>();
    q->rect = rc;
    q->colors[0] = top;
    q->colors[1] = top;
    q->colors[2] = bottom;
    q->colors[3] = bottom;

    e.get_or_create<Display2D>().drawable = std::move(q);
}
}