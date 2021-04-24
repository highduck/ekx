#pragma once

#include <memory>
#include <string>
#include <ek/scenex/text/TextFormat.hpp>
#include <ek/math/box.hpp>
#include <ek/math/mat3x2.hpp>
#include <ek/util/type_index.hpp>
#include <ecxx/ecxx.hpp>
#include <ek/graphics/graphics.hpp>
#include "Sprite.hpp"

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

struct Bounds2D {
//    enum Flags {
//        HitArea,
//        Scissors,
//        Bounds
//    };
    rect_f rect;

    bool hitArea = false;
    bool scissors = false;
    bool culling = false;

    [[nodiscard]]
    rect_f getWorldRect(const matrix_2d& worldMatrix) const;

    [[nodiscard]]
    rect_f getScreenRect(matrix_2d viewMatrix, matrix_2d worldMatrix) const;
};

struct Display2D {
    std::unique_ptr<IDrawable2D> drawable;

    // state management
    Res<graphics::Shader> program;

#ifndef NDEBUG
    bool drawBounds = false;
#endif

    Display2D() = default;

    explicit Display2D(IDrawable2D* ptr) :
            drawable(ptr) {
    }

    template<typename T>
    [[nodiscard]]
    inline T& get() const {
        return *static_cast<T*>(drawable.get());
    }

    template<typename T>
    [[nodiscard]]
    inline T* tryGet() const {
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

    template<typename T>
    static T& make(ecs::EntityApi e) {
        auto& d = e.get_or_create<Display2D>();
        d.drawable = std::move(std::make_unique<T>());
        return static_cast<T&>(*d.drawable);
    }

    template<typename T>
    static T& get(ecs::EntityApi e) {
        return e.get<Display2D>().get<T>();
    }

    template<typename T>
    T& makeDrawable() {
        drawable = std::move(std::make_unique<T>());
        return static_cast<T&>(*drawable);
    }
};

// 16 + 16 = 32 bytes
class Quad2D : public Drawable2D<Quad2D> {
public:
    rect_f rect{0.0f, 0.0f, 1.0f, 1.0f};
    argb32_t colors[4]{};

    void draw() override;

    [[nodiscard]]
    rect_f getBounds() const override;

    [[nodiscard]]
    bool hitTest(float2 point) const override;

    inline Quad2D& setGradientVertical(argb32_t top, argb32_t bottom) {
        colors[0] = colors[1] = top;
        colors[2] = colors[3] = bottom;
        return *this;
    };

    inline Quad2D& setColor(argb32_t color) {
        colors[0] = colors[1] = colors[2] = colors[3] = color;
        return *this;
    };

    inline Quad2D& setHalfExtents(float hw, float hh) {
        rect.set(-hw, -hh, 2.0f * hw, 2.0f * hh);
        return *this;
    }
};

// 8 + 1 = 9 bytes
class Sprite2D : public Drawable2D<Sprite2D> {
public:
    Res<Sprite> src;
    bool hit_pixels = true;

    Sprite2D();

    explicit Sprite2D(const std::string& spriteId);

    void draw() override;

    [[nodiscard]]
    rect_f getBounds() const override;

    [[nodiscard]]
    bool hitTest(float2 point) const override;
};

// 8 + 16 + 16 + 8 + 1 = 49 bytes
class NinePatch2D : public Drawable2D<NinePatch2D> {
public:
    Res<Sprite> src;
    bool hit_pixels = true;

    rect_f scale_grid;
    float2 scale;
    rect_f manual_target;

    NinePatch2D();

    explicit NinePatch2D(const std::string& spriteId, rect_f aScaleGrid = rect_f::zero);

    void draw() override;

    [[nodiscard]]
    rect_f getBounds() const override;

    [[nodiscard]]
    bool hitTest(float2 point) const override;
};

// 8 + 136 + 16 + 8 + 4 = 172 bytes
class Text2D : public Drawable2D<Text2D> {
public:
    std::string text;
    TextFormat format;
    rect_f rect;

    argb32_t borderColor = 0x00FF0000_argb;
    argb32_t fillColor = 0x00000000_argb;

    bool localize = false;

    // Reduce font-size until text fits to the field bounds (if bounds not empty)
    bool adjustsFontSizeToFitBounds = false;

    // if enabled hit test check bounds, otherwise hits visible text bounds
    bool hitFullBounds = true;

    // debug metrics
    bool showTextBounds = false;

    Text2D();

    Text2D(std::string text, TextFormat format);

    void draw() override;

    [[nodiscard]]
    rect_f getBounds() const override;

    [[nodiscard]]
    rect_f getTextBounds() const;

    [[nodiscard]]
    bool hitTest(float2 point) const override;
};

// 4 + 4 + 4 + 4 + 4 + 4 + 8 = 32 bytes
class Arc2D : public Drawable2D<Arc2D> {
public:
    float angle = 0.0f;
    float radius = 10.0f;
    float line_width = 10.0f;
    int segments = 50;
    argb32_t color_inner = argb32_t::one;
    argb32_t color_outer = argb32_t::one;
    Res<Sprite> sprite;

    void draw() override;

    [[nodiscard]]
    rect_f getBounds() const override;

    [[nodiscard]]
    bool hitTest(float2 point) const override;
};


/** utilities **/
void set_gradient_quad(ecs::EntityApi e, const rect_f& rc, argb32_t top, argb32_t bottom);

inline void set_color_quad(ecs::EntityApi e, const rect_f& rc, argb32_t color) {
    set_gradient_quad(e, rc, color, color);
}

template<typename T>
inline T& get_drawable(ecs::EntityApi e) {
    return *static_cast<T*>(e.get<Display2D>().drawable.get());
}

inline void set_text(ecs::EntityApi e, const std::string& v) {
    get_drawable<Text2D>(e).text = v;
}

}


