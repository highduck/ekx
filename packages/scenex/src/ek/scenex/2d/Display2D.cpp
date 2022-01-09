#include "Display2D.hpp"
#include <ek/canvas.h>
#include <ek/scenex/2d/Sprite.hpp>
#include <ek/scenex/text/Font.hpp>
#include <ek/util/Res.hpp>
#include <ek/scenex/text/TextEngine.hpp>
#include <ek/scenex/Localization.hpp>

namespace ek {

IDrawable2D::~IDrawable2D() = default;

void Quad2D::draw() {
    const Sprite* spr = src.get();
    if (spr) {
        const sg_image image = ek_ref_content(sg_image, spr->image_id);
        if (image.id) {
            canvas_set_image_region(image, spr->tex);
        }
    } else {
        canvas_set_empty_image();
    }
    canvas_quad_color4(rect.x, rect.y, rect.w, rect.h, colors[0], colors[1], colors[2], colors[3]);
}

rect_t Quad2D::getBounds() const {
    return rect;
}

bool Quad2D::hitTest(vec2_t point) const {
    return rect_contains(rect, point);
}

/** Sprite2D **/
Sprite2D::Sprite2D() = default;

Sprite2D::Sprite2D(const char* spriteId) :
        src{spriteId} {
}

void Sprite2D::draw() {
    if (!src) {
        return;
    }
    src->draw();
}

rect_t Sprite2D::getBounds() const {
    if (src) {
        return src->rect;
    }
    return (rect_t){};
}

bool Sprite2D::hitTest(vec2_t point) const {
    if (rect_contains(getBounds(), point)) {
        if (hit_pixels && src) {
            return src->hit_test(point);
        }
        return true;
    }
    return false;
}

/** NinePatch2D **/

NinePatch2D::NinePatch2D() = default;

NinePatch2D::NinePatch2D(const char* spriteId, rect_t aScaleGrid) :
        src{spriteId},
        scale_grid{aScaleGrid} {
}

void NinePatch2D::draw() {
    if (!src) {
        return;
    }
    auto& spr = *src;
    canvas_save_matrix();
    canvas_scale({1.0f / scale.x, 1.0f / scale.y});
    // TODO: rotated
    rect_t target = manual_target;
    if (rect_is_empty(target)) {
        target = rect_scale(rect_expand(spr.rect, 1), scale);
    }
    spr.draw_grid(scale_grid, target);
    canvas_restore_matrix();
}

rect_t NinePatch2D::getBounds() const {
    if (src) {
        return src->rect;
    }
    return (rect_t){};
}

bool NinePatch2D::hitTest(vec2_t point) const {
    if (rect_contains(getBounds(), point)) {
        if (hit_pixels && src) {
            return src->hit_test(point);
        }
        return true;
    }
    return false;
}

/** Text2D **/

Text2D::Text2D() : Drawable2D<Text2D>(),
                   text{},
                   format{"mini", 16.0f} {

}

Text2D::Text2D(String text, TextFormat format) : Drawable2D<Text2D>(),
                                                 text{std::move(text)},
                                                 format{format} {
}

float findTextScale(vec2_t textSize, rect_t rc) {
    auto textScale = 1.0f;
    if (rc.w > 0.0f && textSize.x > 0.0f) {
        textScale = std::min(textScale, rc.w / textSize.x);
    }
    if (rc.h > 0.0f && textSize.y > 0.0f) {
        textScale = std::min(textScale, rc.h / textSize.y);
    }
    return textScale;
}

void adjustFontSize(TextEngine& engine, const char* text, rect_t bounds) {
    auto& info = gTextEngine.get().textBlockInfo;
    const float minFontSize = 10.0f;

    engine.format.allowLetterWrap = false;

    engine.getTextSize(text, info);

    if (engine.format.wordWrap) {
        while (engine.format.size > minFontSize &&
               (info.size.x > bounds.w || info.size.y > bounds.h)) {
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
    textDrawer.maxWidth = format.wordWrap ? rect.w : 0.0f;
    if (fillColor.a > 0) {
        canvas_set_empty_image();
        canvas_fill_rect(rect, fillColor);
    }
    if (borderColor.a > 0) {
        canvas_set_empty_image();
        canvas_stroke_rect(rect_expand(rect, 1.0f), borderColor, 1);
    }

    const char* str = localize ? Localization::instance.getText(text.c_str()) : text.c_str();
    if (str == nullptr || *str == '\0') {
        return;
    }

    if (adjustsFontSizeToFitBounds) {
        adjustFontSize(textDrawer, str, rect);
    }

    textDrawer.getTextSize(str, blockInfo);

    const vec2_t position = rect.position + (rect.size - blockInfo.size) * format.alignment;
    textDrawer.position.x = position.x;
    textDrawer.position.y = position.y + blockInfo.lines[0].ascender;
    textDrawer.drawWithBlockInfo(str, blockInfo);

    if (showTextBounds) {
        rect_t bounds;
        bounds.position = position;
        bounds.size = blockInfo.size;
        canvas_stroke_rect(rect_expand(bounds, 1.0f), RGB(0xFF0000), 1);
    }
}

rect_t Text2D::getBounds() const {
    if (hitFullBounds) {
        return rect;
    }
    return getTextBounds();
}

rect_t Text2D::getTextBounds() const {
    auto& textDrawer = gTextEngine.get().engine;
    auto& blockInfo = gTextEngine.get().textBlockInfo;
    textDrawer.format = format;

    const char* str = localize ? Localization::instance.getText(text.c_str()) : text.c_str();

    if (adjustsFontSizeToFitBounds) {
        adjustFontSize(textDrawer, str, rect);
    }

    textDrawer.getTextSize(str, blockInfo);
    const auto textSize = blockInfo.size;

    rect_t textBounds;
    textBounds.position = rect.position + (rect.size - textSize) * format.alignment;
    textBounds.size = textSize;
    return textBounds;
}

bool Text2D::hitTest(vec2_t point) const {
    return rect_contains(getBounds(), point);
}

rect_t Bounds2D::getWorldRect(mat3x2_t world_matrix) const {
    const auto bb = brect_extend_transformed_rect(brect_inf(), rect, world_matrix);
    return brect_get_rect(bb);
}

rect_t Bounds2D::getScreenRect(mat3x2_t viewMatrix, mat3x2_t worldMatrix) const {
    const auto g = mat3x2_mul(viewMatrix, worldMatrix);
    const auto bb = brect_extend_transformed_rect(brect_inf(), rect, g);
    return brect_get_rect(bb);
}

//// arc

void Arc2D::draw() {
    Res<Sprite> f{sprite};
    if (!f) {
        return;
    }

    const sg_image image = ek_ref_content(sg_image, f->image_id);
    if (!image.id) {
        return;
    }

    canvas_set_image(image);
    const rect_t tex = f->tex;
    canvas_set_image_rect({RECT_CENTER_X(tex), tex.y, 0.0f, tex.h});

    float off = -0.5f * MATH_PI;
    canvas_line_arc(0, 0, radius, 0 + off, angle + off, line_width, segments, color_inner, color_outer);
}

rect_t Arc2D::getBounds() const {
    float s = radius + line_width;
    return {-s, -s, 2.0f * s, 2.0f * s};
}

bool Arc2D::hitTest(vec2_t point) const {
    const float len = length_vec2(point);
    return len >= radius && len <= (radius + line_width);
}

/** utilities **/
void set_gradient_quad(ecs::EntityApi e, rect_t rc, rgba_t top, rgba_t bottom) {
    auto q = Pointer<Quad2D>::make();
    q->rect = rc;
    q->colors[0] = top;
    q->colors[1] = top;
    q->colors[2] = bottom;
    q->colors[3] = bottom;

    e.get_or_create<Display2D>().drawable = std::move(q);
}
}