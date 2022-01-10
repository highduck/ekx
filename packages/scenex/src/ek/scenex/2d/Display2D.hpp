#pragma once

#include <ek/ds/String.hpp>
#include <ek/scenex/text/TextFormat.hpp>
#include <ek/util/Type.hpp>
#include <ek/ds/Pointer.hpp>
#include <ecxx/ecxx.hpp>
#include <ek/gfx.h>
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
    virtual bool hitTest(vec2_t point) const = 0;

    [[nodiscard]]
    virtual rect_t getBounds() const = 0;

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
    rect_t rect;

    bool hitArea = false;
    bool scissors = false;
    bool culling = false;

    [[nodiscard]]
    rect_t getWorldRect(mat3x2_t worldMatrix) const;

    [[nodiscard]]
    rect_t getScreenRect(mat3x2_t viewMatrix, mat3x2_t worldMatrix) const;
};

struct Display2D {
    Pointer<IDrawable2D> drawable;

    // state management
    ek_ref(ek_shader) program = {0};

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
    inline bool hitTest(vec2_t local) const {
        return drawable && drawable->hitTest(local);
    }

    [[nodiscard]]
    inline rect_t getBounds() const {
        return drawable ? drawable->getBounds() : rect_t{};
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
    rect_t rect = rect_01();
    color_t colors[4] = {
            COLOR_WHITE,
            COLOR_WHITE,
            COLOR_WHITE,
            COLOR_WHITE,
    };

    void draw() override;

    [[nodiscard]]
    rect_t getBounds() const override;

    [[nodiscard]]
    bool hitTest(vec2_t point) const override;

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

// 8 + 1 = 9 bytes
class Sprite2D : public Drawable2D<Sprite2D> {
public:
    Res<Sprite> src;
    bool hit_pixels = true;

    Sprite2D();

    explicit Sprite2D(const char* spriteId);

    void draw() override;

    [[nodiscard]]
    rect_t getBounds() const override;

    [[nodiscard]]
    bool hitTest(vec2_t point) const override;
};

// 8 + 16 + 16 + 8 + 1 = 49 bytes
class NinePatch2D : public Drawable2D<NinePatch2D> {
public:
    Res<Sprite> src;
    rect_t scale_grid;
    rect_t manual_target = {};
    vec2_t scale;
    bool hit_pixels = true;

    NinePatch2D();

    explicit NinePatch2D(const char* spriteId, rect_t aScaleGrid = {});

    void draw() override;

    [[nodiscard]]
    rect_t getBounds() const override;

    [[nodiscard]]
    bool hitTest(vec2_t point) const override;
};

// 8 + 136 + 16 + 8 + 4 = 172 bytes
class Text2D : public Drawable2D<Text2D> {
public:
    String text;
    TextFormat format;
    rect_t rect = {};

    color_t borderColor = ARGB(0x00FF0000);
    color_t fillColor = ARGB(0x00000000);

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
    rect_t getBounds() const override;

    [[nodiscard]]
    rect_t getTextBounds() const;

    [[nodiscard]]
    bool hitTest(vec2_t point) const override;
};

// 4 + 4 + 4 + 4 + 4 + 4 + 8 = 32 bytes
class Arc2D : public Drawable2D<Arc2D> {
public:
    float angle = 0.0f;
    float radius = 10.0f;
    float line_width = 10.0f;
    int segments = 50;
    color_t color_inner = COLOR_WHITE;
    color_t color_outer = COLOR_WHITE;
    Res<Sprite> sprite;

    void draw() override;

    [[nodiscard]]
    rect_t getBounds() const override;

    [[nodiscard]]
    bool hitTest(vec2_t point) const override;
};


/** utilities **/
void set_gradient_quad(ecs::EntityApi e, rect_t rc, color_t top, color_t bottom);

inline void set_color_quad(ecs::EntityApi e, rect_t rc, color_t color) {
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


