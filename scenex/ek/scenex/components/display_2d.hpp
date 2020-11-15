#pragma once

#include <memory>
#include <string>
#include <ek/scenex/text/text_format.hpp>
#include <ek/math/box.hpp>
#include <ek/math/mat3x2.hpp>
#include <ek/util/type_index.hpp>
#include <ek/scenex/2d/sprite.hpp>

namespace ek {

class drawable_2d_base {
public:
    explicit drawable_2d_base(uint32_t type_id) :
            type_id_{type_id} {
    }

    virtual ~drawable_2d_base();

    virtual void draw() = 0;

    [[nodiscard]]
    virtual bool hit_test(const float2& point) const = 0;

    [[nodiscard]]
    virtual rect_f get_bounds() const = 0;

    [[nodiscard]]
    inline uint32_t get_type_id() const {
        return type_id_;
    }

    template<typename T>
    [[nodiscard]]
    inline bool match_type() {
        return type_id_ == type_index<T, drawable_2d_base>::value;
    }

protected:

    uint32_t type_id_ = 0;
};

template<typename T>
class drawable_2d : public drawable_2d_base {
public:
    explicit drawable_2d() :
            drawable_2d_base{type_index<T, drawable_2d_base>::value} {
    }
};

struct scissors_2d {
    rect_f rect;

    [[nodiscard]]
    rect_f world_rect(const matrix_2d& world_matrix) const;
};

struct hit_area_2d {
    rect_f rect;
};

struct display_2d {
    std::unique_ptr<drawable_2d_base> drawable;
#ifndef NDEBUG
    bool drawBounds = false;
#endif

    display_2d() = default;

    explicit display_2d(drawable_2d_base* ptr) :
            drawable(ptr) {
    }

    template<typename T>
    [[nodiscard]]
    inline T* get() const {
        if (drawable && drawable->match_type<T>()) {
            return static_cast<T*>(drawable.get());
        }
        return nullptr;
    }

    template<typename T>
    [[nodiscard]]
    inline bool is() const {
        return drawable && drawable->match_type<T>();
    }
};

class drawable_quad : public drawable_2d<drawable_quad> {
public:
    rect_f rect{0.0f, 0.0f, 1.0f, 1.0f};
    argb32_t colors[4]{};

//    static CQuad* vGradient(rect_f const& rc, argb32_t top, argb32_t bottom);
//    static CQuad* colored(const rect_f& rc, argb32_t color);

    void draw() override;

    [[nodiscard]]
    rect_f get_bounds() const override;

    [[nodiscard]]
    bool hit_test(const float2& point) const override;

    inline void set_gradient_vertical(argb32_t top, argb32_t bottom) {
        colors[0] = colors[1] = top;
        colors[2] = colors[3] = bottom;
    };
};

class drawable_sprite : public drawable_2d<drawable_sprite> {
public:
    asset_t<sprite_t> src;
    bool hit_pixels = true;

    bool scale_grid_mode = false;
    rect_f scale_grid;
    float2 scale;
    rect_f manual_target;

    drawable_sprite();

    explicit drawable_sprite(const std::string& sprite_id, rect_f a_scale_grid = rect_f::zero);

    void draw() override;

    [[nodiscard]]
    rect_f get_bounds() const override;

// test in local space
    [[nodiscard]]
    bool hit_test(float2 const& point) const override;
};

class drawable_text : public drawable_2d<drawable_text> {
public:
    std::string text;
    TextFormat format;
    rect_f rect;

    argb32_t borderColor = 0x00FF0000_argb;
    argb32_t fillColor = 0x00000000_argb;

    // if enabled hit test check not rect, but visible text bounds
    bool hitFullBounds = true;

    // debug metrics
    bool showTextBounds = false;

    drawable_text();

    drawable_text(std::string text, TextFormat format);

    void draw() override;

    [[nodiscard]]
    rect_f get_bounds() const override;

    [[nodiscard]]
    bool hit_test(const float2& point) const override;
};

class drawable_arc : public drawable_2d<drawable_arc> {
public:
    float angle = 0.0f;
    float radius = 10.0f;
    float line_width = 10.0f;
    int segments = 50;
    argb32_t color_inner = argb32_t::one;
    argb32_t color_outer = argb32_t::one;
    asset_t<sprite_t> sprite;

    void draw() override;

    [[nodiscard]]
    rect_f get_bounds() const override;

    [[nodiscard]]
    bool hit_test(const float2& point) const override;
};

}


