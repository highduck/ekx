#pragma once

#include <ek/math/Matrix3x2.hpp>
#include <ek/math/Vec.hpp>
#include <ek/math/Rect.hpp>
#include <ek/math/Color32.hpp>
#include <ecxx/ecxx.hpp>

namespace ek {

// 32 bytes
struct WorldTransform2D {
    Matrix3x2f matrix{}; // 4 * 6 = 24
    ColorMod32 color{}; // 2 * 4 = 8
};

// TODO: mat2x2, position,
struct Transform2D {
    // 24
    Matrix3x2f matrix{};

    // 8
    ColorMod32 color{};

    // 16
    Vec2f cachedScale = Vec2f::one;
    Vec2f cachedSkew = Vec2f::zero;

    inline void setX(float x) {
        matrix.tx = x;
    }

    inline void setY(float y) {
        matrix.ty = y;
    }

    inline void setMatrix(const Matrix3x2f& m) {
        matrix = m;
        cachedScale = m.scale();
        cachedSkew = m.skew();
    }

    [[nodiscard]]
    inline float getX() const {
        return matrix.tx;
    }

    [[nodiscard]]
    inline float getY() const {
        return matrix.ty;
    }

    inline void translate(float x, float y) {
        matrix.tx += x;
        matrix.ty += y;
    }

    inline void setPosition(Vec2f position_) {
        matrix.tx = position_.x;
        matrix.ty = position_.y;
    }

    [[nodiscard]]
    inline Vec2f getPosition() const {
        return {matrix.tx, matrix.ty};
    }

    inline void setPosition(float x, float y) {
        matrix.tx = x;
        matrix.ty = y;
    }

    inline void setPosition(Vec2f position, Vec2f pivot) {
        const auto xx = -pivot.x;
        const auto yy = -pivot.y;
        matrix.tx = position.x + matrix.a * xx + matrix.c * yy;
        matrix.ty = position.y + matrix.d * yy + matrix.b * xx;
    }

    inline void setPosition(Vec2f position_, Vec2f pivot_, Vec2f origin_) {
        const auto x = position_.x + origin_.x;
        const auto y = position_.y + origin_.y;
        const auto xx = -origin_.x - pivot_.x;
        const auto yy = -origin_.y - pivot_.y;
        matrix.tx = x + matrix.a * xx + matrix.c * yy;
        matrix.ty = y + matrix.d * yy + matrix.b * xx;
    }

    void scale(float factor) {
        cachedScale.x *= factor;
        cachedScale.y *= factor;
        updateMatrix2x2();
    }

    void setScale(float value) {
        cachedScale.x = value;
        cachedScale.y = value;
        updateMatrix2x2();
    }

    void setScale(Vec2f value) {
        cachedScale = value;
        updateMatrix2x2();
    }

    void setScale(float x, float y) {
        cachedScale.x = x;
        cachedScale.y = y;
        updateMatrix2x2();
    }

    inline void setScaleX(float x) {
        cachedScale.x = x;
        updateMatrix2x2();
    }

    inline void setScaleY(float y) {
        cachedScale.y = y;
        updateMatrix2x2();
    }

    [[nodiscard]]
    inline Vec2f getScale() const {
        return cachedScale;
    }

    [[nodiscard]]
    inline float getScaleX() const {
        return cachedScale.x;
    }

    [[nodiscard]]
    inline float getScaleY() const {
        return cachedScale.y;
    }

    void setRotation(float value) {
        cachedSkew.x = value;
        cachedSkew.y = value;
        const auto sn = sinf(value);
        const auto cs = cosf(value);
        matrix.a = cs * cachedScale.x;
        matrix.b = sn * cachedScale.x;
        matrix.c = -sn * cachedScale.y;
        matrix.d = cs * cachedScale.y;
    }

    [[nodiscard]]
    inline float getRotation() const {
//        return skew.x == skew.y ? skew.y : 0.0f;
        return cachedSkew.y;
    }

    [[nodiscard]]
    inline Vec2f getSkew() const {
        return cachedSkew;
    }

    inline void setSkew(Vec2f value) {
        cachedSkew = value;
        updateMatrix2x2();
    }

    inline void setTransform(Vec2f position, Vec2f scale_, Vec2f skew_, Vec2f pivot_) {
        cachedScale = scale_;
        cachedSkew = skew_;
        updateMatrix2x2();
        setPosition(position, pivot_);
    }

    void setTransform(Vec2f position_, Vec2f scale_, float rotation_) {
        cachedScale = scale_;
        cachedSkew.x = rotation_;
        cachedSkew.y = rotation_;
        const auto sn = sinf(rotation_);
        const auto cs = cosf(rotation_);
        matrix.a = cs * cachedScale.x;
        matrix.b = sn * cachedScale.x;
        matrix.c = -sn * cachedScale.y;
        matrix.d = cs * cachedScale.y;
        matrix.tx = position_.x;
        matrix.ty = position_.y;
    }

    inline void setTransform(Vec2f position_, Vec2f scale_, Vec2f skew_) {
        cachedScale = scale_;
        cachedSkew = skew_;
        updateMatrix2x2();
        matrix.tx = position_.x;
        matrix.ty = position_.y;
    }

    void rotate(float value) {
        cachedSkew.x += value;
        cachedSkew.y += value;
        updateMatrix2x2();
    }

    void translate(Vec2f delta) {
        setPosition(getPosition() + delta);
    }

    void lerpScale(Vec2f target, float t) {
        setScale(lerp(getScale(), target, t));
    }

    void lerpPosition(Vec2f target, float t) {
        setPosition(lerp(getPosition(), target, t));
    }

    void lerpRotation(float target, float t) {
        setRotation(Math::lerp(getRotation(), target, t));
    }

    inline void updateMatrix2x2() {
        matrix.a = cosf(cachedSkew.y) * cachedScale.x;
        matrix.b = sinf(cachedSkew.y) * cachedScale.x;
        matrix.c = -sinf(cachedSkew.x) * cachedScale.y;
        matrix.d = cosf(cachedSkew.x) * cachedScale.y;
    }

    static Vec2f transformUp(ecs::EntityApi it, ecs::EntityApi top, Vec2f pos);

    static Vec2f transformDown(ecs::EntityApi top, ecs::EntityApi it, Vec2f pos);

    static Vec2f localToLocal(ecs::EntityApi src, ecs::EntityApi dst, Vec2f pos);

    static Vec2f localToGlobal(ecs::EntityApi local, Vec2f localPos);

    static Vec2f globalToLocal(ecs::EntityApi local, Vec2f globalPos);

    // these functions just helpers and use calculated world matrices, so use it only:
    // - after transform invalidation phase
    // - if node has own Transform2D components
    static void fastLocalToLocal(ecs::EntityApi src, ecs::EntityApi dst, Vec2f pos, Vec2f& out);
};

ECX_TYPE(14, WorldTransform2D);
ECX_TYPE(15, Transform2D);

/** system to invalidate matrix and color in world space **/
//void updateWorldTransform2D(ecs::EntityApi root);
//void updateWorldTransformAll(ecs::World* w, ecs::EntityApi root);

void updateWorldTransformAll2(ecs::World* w, ecs::EntityApi root);

/** utility functions **/

inline void setAlpha(ecs::EntityApi e, float alpha) {
    e.get_or_create<Transform2D>().color.setAlpha(alpha);
}

inline void setColorScale(ecs::EntityApi e, argb32_t color_multiplier) {
    e.get_or_create<Transform2D>().color.scale = color_multiplier;
}

inline void setColorOffset(ecs::EntityApi e, argb32_t color_offset) {
    e.get_or_create<Transform2D>().color.offset = color_offset;
}

inline void setPosition(ecs::EntityApi e, Vec2f pos) {
    e.get_or_create<Transform2D>().setPosition(pos);
}

inline Vec2f getPosition(const ecs::EntityApi e) {
    return e.get_or_default<Transform2D>().getPosition();
}

inline void setRotation(ecs::EntityApi e, float radians) {
    e.get_or_create<Transform2D>().setRotation(radians);
}

inline float getRotation(const ecs::EntityApi e) {
    return e.get_or_default<Transform2D>().getRotation();
}

inline void setScale(ecs::EntityApi e, Vec2f sc) {
    e.get_or_create<Transform2D>().setScale(sc);
}

inline void setScale(ecs::EntityApi e, float xy) {
    e.get_or_create<Transform2D>().setScale(xy, xy);
}

inline Vec2f getScale(ecs::EntityApi e) {
    return e.get_or_default<Transform2D>().getScale();
}

}


