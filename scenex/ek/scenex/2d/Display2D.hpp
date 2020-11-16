#pragma once

#include <memory>
#include <string>
#include <ek/scenex/text/text_format.hpp>
#include <ek/math/box.hpp>
#include <ek/math/mat3x2.hpp>
#include <ek/util/type_index.hpp>
#include <ek/scenex/2d/sprite.hpp>

namespace ek {

class IDrawable2D {
public:
    explicit IDrawable2D(uint32_t type_id) :
            typeID_{type_id} {
    }

    virtual ~IDrawable2D();

    virtual void draw() = 0;

    [[nodiscard]]
    virtual bool hitTest(float2 point) const = 0;

    [[nodiscard]]
    virtual rect_f getBounds() const = 0;

    [[nodiscard]]
    inline uint32_t getTypeID() const {
        return typeID_;
    }

    template<typename T>
    [[nodiscard]] inline bool matchType() {
        return typeID_ == type_index<T, IDrawable2D>::value;
    }

protected:

    uint32_t typeID_ = 0;
};

template<typename T>
class Drawable2D : public IDrawable2D {
public:
    Drawable2D() :
            IDrawable2D{type_index<T, IDrawable2D>::value} {
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

struct Display2D {
    std::unique_ptr<IDrawable2D> drawable;
#ifndef NDEBUG
    bool drawBounds = false;
#endif

    Display2D() = default;

    explicit Display2D(IDrawable2D* ptr) :
            drawable(ptr) {
    }

    template<typename T>
    [[nodiscard]]
    inline T* get() const {
        if (drawable && drawable->matchType<T>()) {
            return static_cast<T*>(drawable.get());
        }
        return nullptr;
    }

    template<typename T>
    [[nodiscard]]
    inline bool is() const {
        return drawable && drawable->matchType<T>();
    }

    [[nodiscard]]
    inline bool hitTest(float2 local) const {
        return drawable && drawable->hitTest(local);
    }

    [[nodiscard]]
    inline rect_f getBounds() const {
        return drawable ? drawable->getBounds() : rect_f{};
    }
};

class Quad2D : public Drawable2D<Quad2D> {
public:
    rect_f rect{0.0f, 0.0f, 1.0f, 1.0f};
    argb32_t colors[4]{};

//    static CQuad* vGradient(rect_f const& rc, argb32_t top, argb32_t bottom);
//    static CQuad* colored(const rect_f& rc, argb32_t color);

    void draw() override;

    [[nodiscard]]
    rect_f getBounds() const override;

    [[nodiscard]]
    bool hitTest(float2 point) const override;

    inline void setGradientVertical(argb32_t top, argb32_t bottom) {
        colors[0] = colors[1] = top;
        colors[2] = colors[3] = bottom;
    };
};

class Sprite2D : public Drawable2D<Sprite2D> {
public:
    asset_t<sprite_t> src;
    bool hit_pixels = true;

    bool scale_grid_mode = false;
    rect_f scale_grid;
    float2 scale;
    rect_f manual_target;

    Sprite2D();

    explicit Sprite2D(const std::string& sprite_id, rect_f a_scale_grid = rect_f::zero);

    void draw() override;

    [[nodiscard]]
    rect_f getBounds() const override;

    [[nodiscard]]
    bool hitTest(float2 point) const override;
};

class Text2D : public Drawable2D<Text2D> {
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

    Text2D();

    Text2D(std::string text, TextFormat format);

    void draw() override;

    [[nodiscard]]
    rect_f getBounds() const override;

    [[nodiscard]]
    bool hitTest(float2 point) const override;
};

class Arc2D : public Drawable2D<Arc2D> {
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
    rect_f getBounds() const override;

    [[nodiscard]]
    bool hitTest(float2 point) const override;
};

}


