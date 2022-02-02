#include "Display2D.hpp"
#include <ek/canvas.h>
#include <ek/scenex/2d/Sprite.hpp>
#include <ek/scenex/text/Font.hpp>

#include <ek/scenex/text/TextEngine.hpp>
#include <ekx/app/localization.h>

namespace ek {

void quad2d_draw(entity_t e) {
    auto& d = ecs::EntityApi{e}.get<Quad2D>();
    const sprite_t* spr = &REF_RESOLVE(res_sprite, d.src);
    if (spr->state & SPRITE_LOADED) {
        const sg_image image = REF_RESOLVE(res_image, spr->image_id);
        if (image.id) {
            canvas_set_image_region(image, spr->tex);
        }
    } else {
        canvas_set_empty_image();
    }
    auto rc = d.rect;
    auto colors = d.colors;
    canvas_quad_color4(rc.x, rc.y, rc.w, rc.h, colors[0], colors[1], colors[2], colors[3]);
}

rect_t quad2d_get_bounds(entity_t e) {
    return ecs::EntityApi{e}.get<Quad2D>().rect;
}

bool quad2d_hit_test(entity_t e, vec2_t lp) {
    return rect_contains(ecs::EntityApi{e}.get<Quad2D>().rect, lp);
}

/** Sprite2D **/
Sprite2D::Sprite2D() = default;

Sprite2D::Sprite2D(string_hash_t spriteId) :
        src{R_SPRITE(spriteId)} {
}

void sprite2d_draw(entity_t e) {
    auto src = ecs::EntityApi{e}.get<Sprite2D>().src;
    if (!src) {
        return;
    }
    const sprite_t* sprite = &REF_RESOLVE(res_sprite, src);
    ::ek::draw(sprite);
}

rect_t sprite2d_get_bounds(entity_t e) {
    auto src = ecs::EntityApi{e}.get<Sprite2D>().src;
    if (src) {
        const sprite_t* sprite = &REF_RESOLVE(res_sprite, src);
        return sprite->rect;
    }
    return (rect_t) {};
}

bool sprite2d_hit_test(entity_t e, vec2_t point) {
    if (rect_contains(sprite2d_get_bounds(e), point)) {
        auto& d = ecs::EntityApi{e}.get<Sprite2D>();
        if (d.hit_pixels && d.src) {
            const sprite_t* sprite = &REF_RESOLVE(res_sprite, d.src);
            return ::ek::hit_test(sprite, point);
        }
        return true;
    }
    return false;
}

/** NinePatch2D **/

NinePatch2D::NinePatch2D() = default;

NinePatch2D::NinePatch2D(string_hash_t spriteId, rect_t aScaleGrid) :
        src{R_SPRITE(spriteId)},
        scale_grid{aScaleGrid} {
}

void ninepatch2d_draw(entity_t e) {
    const auto& d = ecs::EntityApi{e}.get<NinePatch2D>();
    if (!d.src) {
        return;
    }
    auto* spr = &REF_RESOLVE(res_sprite, d.src);
    canvas_save_matrix();
    canvas_scale(1.0f / d.scale);
    // TODO: rotated
    rect_t target = d.manual_target;
    if (rect_is_empty(target)) {
        target = rect_scale(rect_expand(spr->rect, -1), d.scale);
    }
    ek::draw_grid(spr, d.scale_grid, target);
    canvas_restore_matrix();
}

rect_t ninepatch2d_get_bounds(entity_t e) {
    auto src = ecs::EntityApi{e}.get<NinePatch2D>().src;
    if (src) {
        return REF_RESOLVE(res_sprite, src).rect;
    }
    return (rect_t) {};
}

bool ninepatch2d_hit_test(entity_t e, vec2_t point) {
    if (rect_contains(ninepatch2d_get_bounds(e), point)) {
        const auto& d = ecs::EntityApi{e}.get<NinePatch2D>();
        if (d.hit_pixels && d.src) {
            return hit_test(&REF_RESOLVE(res_sprite, d.src), point);
        }
        return true;
    }
    return false;
}

/** Text2D **/

Text2D::Text2D() : text{},
                   format{H("mini"), 16.0f} {

}

Text2D::Text2D(String text, TextFormat format) : text{std::move(text)},
                                                 format{format} {
}

float findTextScale(vec2_t textSize, rect_t rc) {
    auto textScale = 1.0f;
    if (rc.w > 0.0f && textSize.x > 0.0f) {
        textScale = MIN(textScale, rc.w / textSize.x);
    }
    if (rc.h > 0.0f && textSize.y > 0.0f) {
        textScale = MIN(textScale, rc.h / textSize.y);
    }
    return textScale;
}

void adjustFontSize(TextEngine& engine, const char* text, rect_t bounds) {
    auto& info = get_text_engine()->textBlockInfo;
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

void text2d_draw(entity_t e) {
    auto& d = ecs::EntityApi{e}.get<Text2D>();
    auto& textDrawer = get_text_engine()->engine;
    auto& blockInfo = get_text_engine()->textBlockInfo;
    textDrawer.format = d.format;
    textDrawer.maxWidth = d.format.wordWrap ? d.rect.w : 0.0f;
    if (d.fillColor.a > 0) {
        canvas_set_empty_image();
        canvas_fill_rect(d.rect, d.fillColor);
    }
    if (d.borderColor.a > 0) {
        canvas_set_empty_image();
        canvas_stroke_rect(rect_expand(d.rect, 1.0f), d.borderColor, 1);
    }

    const char* str = d.localized ? localize(d.text.c_str()) : d.text.c_str();
    if (str == nullptr || *str == '\0') {
        return;
    }

    if (d.adjustsFontSizeToFitBounds) {
        adjustFontSize(textDrawer, str, d.rect);
    }

    textDrawer.getTextSize(str, blockInfo);

    const vec2_t position = d.rect.position + (d.rect.size - blockInfo.size) * d.format.alignment;
    textDrawer.position.x = position.x;
    textDrawer.position.y = position.y + blockInfo.lines[0].ascender;
    textDrawer.drawWithBlockInfo(str, blockInfo);

    if (d.showTextBounds) {
        rect_t bounds;
        bounds.position = position;
        bounds.size = blockInfo.size;
        canvas_stroke_rect(rect_expand(bounds, 1.0f), RGB(0xFF0000), 1);
    }
}

rect_t text2d_get_bounds(entity_t e) {
    auto& d = ecs::EntityApi{e}.get<Text2D>();
    if (d.hitFullBounds) {
        return d.rect;
    }
    return d.getTextBounds();
}

rect_t Text2D::getTextBounds() const {
    auto& textDrawer = get_text_engine()->engine;
    auto& blockInfo = get_text_engine()->textBlockInfo;
    textDrawer.format = format;

    const char* str = localized ? localize(text.c_str()) : text.c_str();

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

bool text2d_hit_test(entity_t e, vec2_t point) {
    return rect_contains(text2d_get_bounds(e), point);
}

rect_t Bounds2D::getWorldRect(mat3x2_t world_matrix) const {
    const auto bb = aabb2_add_transformed_rect(aabb2_empty(), rect, world_matrix);
    return aabb2_get_rect(bb);
}

rect_t Bounds2D::getScreenRect(mat3x2_t viewMatrix, mat3x2_t worldMatrix) const {
    const auto g = mat3x2_mul(viewMatrix, worldMatrix);
    const auto bb = aabb2_add_transformed_rect(aabb2_empty(), rect, g);
    return aabb2_get_rect(bb);
}

//// arc

void arc2d_draw(entity_t e) {
    auto* d = ecs::C<Arc2D>::get_by_entity(e);
    if (!d->sprite) {
        return;
    }

    const sprite_t* spr = &REF_RESOLVE(res_sprite, d->sprite);
    const sg_image image = REF_RESOLVE(res_image, spr->image_id);
    if (!image.id) {
        return;
    }

    canvas_set_image(image);
    const rect_t tex = spr->tex;
    canvas_set_image_rect({RECT_CENTER_X(tex), tex.y, 0.0f, tex.h});

    float off = -0.5f * MATH_PI;
    canvas_line_arc(0, 0, d->radius,
                    0 + off, d->angle + off,
                    d->line_width, d->segments,
                    d->color_inner, d->color_outer);
}

rect_t arc2d_get_bounds(entity_t e) {
    auto* d = ecs::C<Arc2D>::get_by_entity(e);
    float s = d->radius + d->line_width;
    return {-s, -s, 2.0f * s, 2.0f * s};
}

bool arc2d_hit_test(entity_t e, vec2_t point) {
    auto* d = ecs::C<Arc2D>::get_by_entity(e);
    const float len = length_vec2(point);
    const float r = d->radius;
    const float dr = d->line_width;
    return len >= r && len <= (r + dr);
}

/** utilities **/
void set_gradient_quad(ecs::EntityApi e, rect_t rc, color_t top, color_t bottom) {
    auto& q = e.get_or_create<Quad2D>();
    q.rect = rc;
    q.colors[0] = top;
    q.colors[1] = top;
    q.colors[2] = bottom;
    q.colors[3] = bottom;

    auto& display = e.get_or_create<Display2D>();
    display.draw = quad2d_draw;
    display.hit_test = quad2d_hit_test;
    display.get_bounds = quad2d_get_bounds;
}


Quad2D* quad2d_setup(entity_t e) {
    auto entity = ecs::EntityApi{e};
    Display2D& display = entity.get_or_create<Display2D>();
    Quad2D& drawable = entity.get_or_create<Quad2D>();
    display.draw = quad2d_draw;
    display.hit_test = quad2d_hit_test;
    display.get_bounds = quad2d_get_bounds;
    return &drawable;
}

Sprite2D* sprite2d_setup(entity_t e) {
    auto entity = ecs::EntityApi{e};
    Display2D& display = entity.get_or_create<Display2D>();
    Sprite2D& drawable = entity.get_or_create<Sprite2D>();
    display.draw = sprite2d_draw;
    display.hit_test = sprite2d_hit_test;
    display.get_bounds = sprite2d_get_bounds;
    return &drawable;
}

NinePatch2D* ninepatch2d_setup(entity_t e) {
    auto entity = ecs::EntityApi{e};
    Display2D& display = entity.get_or_create<Display2D>();
    NinePatch2D& drawable = entity.get_or_create<NinePatch2D>();
    display.draw = ninepatch2d_draw;
    display.hit_test = ninepatch2d_hit_test;
    display.get_bounds = ninepatch2d_get_bounds;
    return &drawable;
}

Text2D* text2d_setup(entity_t e) {
    auto entity = ecs::EntityApi{e};
    Display2D& display = entity.get_or_create<Display2D>();
    Text2D& drawable = entity.get_or_create<Text2D>();
    display.draw = text2d_draw;
    display.hit_test = text2d_hit_test;
    display.get_bounds = text2d_get_bounds;
    return &drawable;
}

Text2D* text2d_setup_ex(entity_t e, const String& text, TextFormat format) {
    Text2D* d = text2d_setup(e);
    d->text = text;
    d->format = format;
    return d;
}

Arc2D* arc2d_setup(entity_t e) {
    auto entity = ecs::EntityApi{e};
    Display2D& display = entity.get_or_create<Display2D>();
    Arc2D& drawable = entity.get_or_create<Arc2D>();
    display.draw = arc2d_draw;
    display.hit_test = arc2d_hit_test;
    display.get_bounds = arc2d_get_bounds;
    return &drawable;
}

}