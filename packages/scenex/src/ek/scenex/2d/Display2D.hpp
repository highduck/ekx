#pragma once

#include <ek/gfx.h>
#include <ek/log.h>
#include <ek/assert.h>
#include <ek/ds/String.hpp>
#include <ek/scenex/text/TextFormat.hpp>
#include <ecxx/ecxx.hpp>
#include "Sprite.hpp"

namespace ek {

enum Bounds2DFlags {
    BOUNDS_2D_HIT_AREA = 1,
    BOUNDS_2D_SCISSORS = 2,
    BOUNDS_2D_CULL = 4,
};

struct Bounds2D {
    rect_t rect = rect_01();
    uint32_t flags = 0;

    [[nodiscard]]
    rect_t getWorldRect(mat3x2_t worldMatrix) const;

    [[nodiscard]]
    rect_t getScreenRect(mat3x2_t viewMatrix, mat3x2_t worldMatrix) const;
};

struct Display2D {
    // state management
    R(ek_shader) program = 0;

    // 1 - draw debug bounds
    uint32_t flags = 0;

    void (* draw)(entity_t e) = nullptr;

    // TODO: rename as post draw
    void (* callback)(entity_t e) = nullptr;

    bool (* hit_test)(entity_t e, vec2_t local_pos) = nullptr;

    rect_t (* get_bounds)(entity_t e) = nullptr;
};

// 16 + 16 = 32 bytes
class Quad2D {
public:
    R(sprite_t) src = R_SPRITE_EMPTY;
    rect_t rect = rect_01();
    color_t colors[4] = {
            COLOR_WHITE,
            COLOR_WHITE,
            COLOR_WHITE,
            COLOR_WHITE,
    };

    inline Quad2D& setGradientVertical(color_t top, color_t bottom) {
        colors[0] = colors[1] = top;
        colors[2] = colors[3] = bottom;
        return *this;
    };

    inline Quad2D& setColor(color_t color) {
        colors[0] = colors[1] = colors[2] = colors[3] = color;
        return *this;
    };

    inline Quad2D& setHalfExtents(float hw, float hh) {
        this->rect = ::rect(-hw, -hh, 2.0f * hw, 2.0f * hh);
        return *this;
    }
};

void quad2d_draw(entity_t e);

rect_t quad2d_get_bounds(entity_t e);

bool quad2d_hit_test(entity_t e, vec2_t lp);

void sprite2d_draw(entity_t e);

rect_t sprite2d_get_bounds(entity_t e);

bool sprite2d_hit_test(entity_t e, vec2_t lp);

void ninepatch2d_draw(entity_t e);

rect_t ninepatch2d_get_bounds(entity_t e);

bool ninepatch2d_hit_test(entity_t e, vec2_t lp);

void text2d_draw(entity_t e);

rect_t text2d_get_bounds(entity_t e);

bool text2d_hit_test(entity_t e, vec2_t lp);

void arc2d_draw(entity_t e);

rect_t arc2d_get_bounds(entity_t e);

bool arc2d_hit_test(entity_t e, vec2_t lp);


// 8 + 1 = 9 bytes
class Sprite2D {
public:
    R(sprite_t) src = 0;
    bool hit_pixels = true;

    Sprite2D();

    explicit Sprite2D(string_hash_t spriteId);
};

// 8 + 16 + 16 + 8 + 1 = 49 bytes
class NinePatch2D {
public:
    R(sprite_t) src = 0;
    rect_t scale_grid;
    rect_t manual_target = {};
    vec2_t scale;
    bool hit_pixels = true;

    NinePatch2D();

    explicit NinePatch2D(string_hash_t spriteId, rect_t aScaleGrid = {});
};

enum text2d_flags {
    TEXT2D_C_STR = 0,
    TEXT2D_INPLACE = 1,
    TEXT2D_STR_BUF = 2,
    TEXT2D_STR_MASK = 0x3,
    TEXT2D_LOCALIZED = 4,

};

// 8 + 136 + 16 + 8 + 4 = 172 bytes
struct Text2D {
    // 3 modes for string storage:
    // 0 - const c-string
    // 1 - inplace buffer
    // 2 - dynamic string buffer
    char buffer[32];
    String str_buf;
    const char* c_str;

    uint32_t flags = 0;

    TextFormat format;
    rect_t rect = {};

    color_t borderColor = ARGB(0x00FF0000);
    color_t fillColor = ARGB(0x00000000);

    bool localized = false;

    // Reduce font-size until text fits to the field bounds (if bounds not empty)
    bool adjustsFontSizeToFitBounds = false;

    // if enabled hit test check bounds, otherwise hits visible text bounds
    bool hitFullBounds = true;

    // debug metrics
    bool showTextBounds = false;

    Text2D();

    Text2D(String text, TextFormat format);

    [[nodiscard]]
    rect_t getTextBounds() const;
};

inline const char* text2d__c_str(const Text2D* text2d) {
    switch(text2d->flags & TEXT2D_STR_MASK) {
        case TEXT2D_C_STR: return text2d->c_str;
        case TEXT2D_INPLACE: return text2d->buffer;
        case TEXT2D_STR_BUF: return text2d->str_buf.c_str();
    }
    // non-reachable
    return "";
}

inline void text2d__set_str_mode(Text2D* text2d, uint32_t mode) {
    text2d->flags = ((text2d->flags >> 2u) << 2u) | mode;
}

// 4 + 4 + 4 + 4 + 4 + 4 + 8 = 32 bytes
class Arc2D {
public:
    float angle = 0.0f;
    float radius = 10.0f;
    float line_width = 10.0f;
    int segments = 50;
    color_t color_inner = COLOR_WHITE;
    color_t color_outer = COLOR_WHITE;
    R(sprite_t) sprite = 0;
};

/** utilities **/
void set_gradient_quad(ecs::EntityApi e, rect_t rc, color_t top, color_t bottom);

inline void set_color_quad(ecs::EntityApi e, rect_t rc, color_t color) {
    set_gradient_quad(e, rc, color, color);
}

inline void set_text(entity_t e, const char* cstr) {
    auto* txt = ecs::tryGet<Text2D>(e);
    if (txt) {
        txt->c_str = cstr;
        text2d__set_str_mode(txt, 0);
    }
}

inline void set_text_timer(entity_t e, int millis, uint32_t flags) {
    auto* txt = ecs::tryGet<Text2D>(e);
    if (txt) {
        ek_cstr_format_timer(txt->buffer, sizeof(txt->buffer), millis, flags);
        text2d__set_str_mode(txt, TEXT2D_INPLACE);
    }
}

inline void set_text_dynamic(entity_t e, const String& v) {
    auto* txt = ecs::tryGet<Text2D>(e);
    if (txt) {
        txt->str_buf = v;
        text2d__set_str_mode(txt, TEXT2D_STR_BUF);
    }
}

inline void setTextF(uint32_t e, const char* fmt, ...) {
    va_list va;
    va_start(va, fmt);

    auto* txt = ecs::tryGet<Text2D>(e);
    if (txt) {
        ek_vsnprintf(txt->buffer, sizeof(txt->buffer), fmt, va);
        text2d__set_str_mode(txt, TEXT2D_INPLACE);
    }

    va_end(va);
}

inline void copy_entity_text(entity_t to, entity_t from) {
    auto* src = ecs::tryGet<Text2D>(from);
    auto* dst = ecs::tryGet<Text2D>(to);
    if (src && dst) {
        dst->flags = ((dst->flags >> 2u) << 2u) | (src->flags & TEXT2D_STR_MASK);
        dst->c_str = src->c_str;
        dst->str_buf = src->str_buf;
        memcpy(dst->buffer, src->buffer, sizeof(src->buffer));
    }
}

Quad2D* quad2d_setup(entity_t e);

Sprite2D* sprite2d_setup(entity_t e);

NinePatch2D* ninepatch2d_setup(entity_t e);

Text2D* text2d_setup(entity_t e);

Text2D* text2d_setup_ex(entity_t e, TextFormat format);

Arc2D* arc2d_setup(entity_t e);

}

ECX_COMP_TYPE_CXX(ek::Text2D)
