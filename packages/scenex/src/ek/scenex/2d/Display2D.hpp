#pragma once

#include <ek/ds/String.hpp>
#include <ek/scenex/text/TextFormat.hpp>
#include <ek/math/Rect.hpp>
#include <ek/math/Matrix3x2.hpp>
#include <ek/util/Type.hpp>
#include <ek/ds/Pointer.hpp>
#include <ecxx/ecxx.hpp>
#include <ek/graphics/graphics.hpp>
#include "Sprite.hpp"
#include <ek/log.h>
#include <ek/assert.h>
#include <ek/scenex/types.h>

namespace ek {

class IDrawable2D {
public:
    IDrawable2D() = delete;

    constexpr explicit IDrawable2D(int type_id) noexcept: typeID_{type_id} {
    }

    virtual ~IDrawable2D();

    virtual void draw() = 0;

    [[nodiscard]]
    virtual bool hitTest(Vec2f point) const = 0;

    [[nodiscard]]
    virtual Rect2f getBounds() const = 0;

    [[nodiscard]]
    uint32_t getTypeID() const {
        return typeID_;
    }

    template<typename T>
    [[nodiscard]] bool matchType() const {
        return typeID_ == TypeIndex<T, IDrawable2D>::value;
    }

protected:

    int typeID_;
};

template<typename T>
class Drawable2D : public IDrawable2D {
public:
    Drawable2D() : IDrawable2D{TypeIndex<T, IDrawable2D>::value} {
    }

    ~Drawable2D() override = default;
};

struct Bounds2D {
//    enum Flags {
//        HitArea,
//        Scissors,
//        Bounds
//    };
    Rect2f rect;

    bool hitArea = false;
    bool scissors = false;
    bool culling = false;

    [[nodiscard]]
    Rect2f getWorldRect(const Matrix3x2f& worldMatrix) const;

    [[nodiscard]]
    Rect2f getScreenRect(Matrix3x2f viewMatrix, Matrix3x2f worldMatrix) const;
};

struct Display2D {
    Pointer<IDrawable2D> drawable;

    // state management
    Res<Shader> program;

    // 1 - draw debug bounds
    int flags = 0;

    Display2D() = default;

    explicit Display2D(IDrawable2D* ptr) : drawable(ptr) {
    }

    template<typename T>
    [[nodiscard]]
    inline T& get() const {
        if (!drawable) {
            EK_WARN("Drawable2D required");
        }
        if (!drawable->matchType<T>()) {
            EK_WARN("Drawable2D TypeID mismatch: required %d, got %d", TypeIndex<T, IDrawable2D>::value,
                      drawable->getTypeID());
        }
        EK_ASSERT(!!drawable);
        EK_ASSERT(drawable->matchType<T>());
        return *(T*) drawable.get();
    }

    template<typename T>
    [[nodiscard]]
    inline T* tryGet() const {
        if (drawable && drawable->matchType<T>()) {
            return (T*) drawable.get();
        }
        return nullptr;
    }

    template<typename T>
    [[nodiscard]]
    inline bool is() const {
        return drawable && drawable->matchType<T>();
    }

    [[nodiscard]]
    inline bool hitTest(Vec2f local) const {
        return drawable && drawable->hitTest(local);
    }

    [[nodiscard]]
    inline Rect2f getBounds() const {
        return drawable ? drawable->getBounds() : Rect2f{};
    }

    template<typename T, typename ...Args>
    inline static T& make(ecs::EntityApi e, Args&& ...args) {
        auto& d = e.get_or_create<Display2D>();
        d.drawable = std::move(Pointer<T>::make(args...));
        return static_cast<T&>(*d.drawable);
    }

    template<typename T>
    static T& get(ecs::EntityApi e) {
        auto* display = e.tryGet<Display2D>();
        if (!display) {
            EK_WARN("Display2D required");
        }
        return display->get<T>();
    }

    template<typename T>
    static T* tryGet(ecs::EntityApi e) {
        auto* display = e.tryGet<Display2D>();
        return display ? display->tryGet<T>() : nullptr;
    }

    template<typename T>
    T& makeDrawable() {
        drawable = Pointer<T>::make();
        return static_cast<T&>(*drawable);
    }
};

// 16 + 16 = 32 bytes
class Quad2D : public Drawable2D<Quad2D> {
public:
    Res<Sprite> src{"empty"};
    Rect2f rect{0.0f, 0.0f, 1.0f, 1.0f};
    argb32_t colors[4]{argb32_t::one, argb32_t::one, argb32_t::one, argb32_t::one};

    void draw() override;

    [[nodiscard]]
    Rect2f getBounds() const override;

    [[nodiscard]]
    bool hitTest(Vec2f point) const override;

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

    explicit Sprite2D(const char* spriteId);

    void draw() override;

    [[nodiscard]]
    Rect2f getBounds() const override;

    [[nodiscard]]
    bool hitTest(Vec2f point) const override;
};

// 8 + 16 + 16 + 8 + 1 = 49 bytes
class NinePatch2D : public Drawable2D<NinePatch2D> {
public:
    Res<Sprite> src;
    bool hit_pixels = true;

    Rect2f scale_grid;
    Vec2f scale;
    Rect2f manual_target;

    NinePatch2D();

    explicit NinePatch2D(const char* spriteId, Rect2f aScaleGrid = Rect2f::zero);

    void draw() override;

    [[nodiscard]]
    Rect2f getBounds() const override;

    [[nodiscard]]
    bool hitTest(Vec2f point) const override;
};

// 8 + 136 + 16 + 8 + 4 = 172 bytes
class Text2D : public Drawable2D<Text2D> {
public:
    String text;
    TextFormat format;
    Rect2f rect;

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

    Text2D(String text, TextFormat format);

    void draw() override;

    [[nodiscard]]
    Rect2f getBounds() const override;

    [[nodiscard]]
    Rect2f getTextBounds() const;

    [[nodiscard]]
    bool hitTest(Vec2f point) const override;
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
    Rect2f getBounds() const override;

    [[nodiscard]]
    bool hitTest(Vec2f point) const override;
};


/** utilities **/
void set_gradient_quad(ecs::EntityApi e, const Rect2f& rc, argb32_t top, argb32_t bottom);

inline void set_color_quad(ecs::EntityApi e, const Rect2f& rc, argb32_t color) {
    set_gradient_quad(e, rc, color, color);
}

template<typename T>
inline T& getDrawable(ecs::EntityApi e) {
    return e.get<Display2D>().get<T>();
}

inline void setText(ecs::EntityApi e, const String& v) {
    auto* d = e.tryGet<Display2D>();
    if (d) {
        auto* txt = d->tryGet<Text2D>();
        if (txt) {
            txt->text = v;
        }
    }
}

ECX_TYPE(11, Display2D);
ECX_TYPE(32, Bounds2D);

EK_DECLARE_TYPE(IDrawable2D);
EK_TYPE_INDEX_T(IDrawable2D, Sprite2D, IDrawable2D_Sprite2D);
EK_TYPE_INDEX_T(IDrawable2D, Quad2D, IDrawable2D_Quad2D);
EK_TYPE_INDEX_T(IDrawable2D, Arc2D, IDrawable2D_Arc2D);
EK_TYPE_INDEX_T(IDrawable2D, NinePatch2D, IDrawable2D_NinePatch2D);
EK_TYPE_INDEX_T(IDrawable2D, Text2D, IDrawable2D_Text2D);

}


