#pragma once

#include <ek/math/mat3x2.hpp>
#include <ek/math/vec.hpp>
#include <ek/math/box.hpp>
#include <ek/math/packed_color.hpp>
#include <ecxx/ecxx.hpp>

namespace ek {

// 32 bytes
struct WorldTransform2D {
    alignas(16) matrix_2d matrix{}; // 4 * 6 = 24
    ColorMod32 color{}; // 2 * 4 = 8
};

// TODO: mat2x2, position,
struct Transform2D {
    // 24
    alignas(16) matrix_2d matrix{};

    // 8
    ColorMod32 color{};

    // 16
    float2 cachedScale = float2::one;
    float2 cachedSkew = float2::zero;

    inline void setX(float x) {
        matrix.tx = x;
    }

    inline void setY(float y) {
        matrix.ty = y;
    }

    inline void setMatrix(const matrix_2d& m) {
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

    inline void setPosition(float2 position_) {
        matrix.tx = position_.x;
        matrix.ty = position_.y;
    }

    [[nodiscard]]
    inline float2 getPosition() const {
        return {matrix.tx, matrix.ty};
    }

    inline void setPosition(float x, float y) {
        matrix.tx = x;
        matrix.ty = y;
    }

    inline void setPosition(float2 position, float2 pivot) {
        const auto xx = -pivot.x;
        const auto yy = -pivot.y;
        matrix.tx = position.x + matrix.a * xx + matrix.c * yy;
        matrix.ty = position.y + matrix.d * yy + matrix.b * xx;
    }

    inline void setPosition(float2 position_, float2 pivot_, float2 origin_) {
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

    void setScale(float2 value) {
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
    inline float2 getScale() const {
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
    inline float2 getSkew() const {
        return cachedSkew;
    }

    inline void setSkew(float2 value) {
        cachedSkew = value;
        updateMatrix2x2();
    }

    inline void setTransform(float2 position, float2 scale_, float2 skew_, float2 pivot_) {
        cachedScale = scale_;
        cachedSkew = skew_;
        updateMatrix2x2();
        setPosition(position, pivot_);
    }

    void setTransform(float2 position_, float2 scale_, float rotation_) {
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

    inline void setTransform(float2 position_, float2 scale_, float2 skew_) {
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

    void translate(float2 delta) {
        setPosition(getPosition() + delta);
    }

    void lerpScale(float2 target, float t) {
        setScale(lerp(getScale(), target, t));
    }

    void lerpPosition(float2 target, float t) {
        setPosition(lerp(getPosition(), target, t));
    }

    void lerpRotation(float target, float t) {
        setRotation(math::lerp(getRotation(), target, t));
    }

    inline void updateMatrix2x2() {
        matrix.a = cosf(cachedSkew.y) * cachedScale.x;
        matrix.b = sinf(cachedSkew.y) * cachedScale.x;
        matrix.c = -sinf(cachedSkew.x) * cachedScale.y;
        matrix.d = cosf(cachedSkew.x) * cachedScale.y;
    }

    static float2 transformUp(ecs::EntityApi it, ecs::EntityApi top, float2 pos);

    static float2 transformDown(ecs::EntityApi top, ecs::EntityApi it, float2 pos);

    static float2 localToLocal(ecs::EntityApi src, ecs::EntityApi dst, float2 pos);

    static float2 localToGlobal(ecs::EntityApi local, float2 localPos);

    static float2 globalToLocal(ecs::EntityApi local, float2 globalPos);

    // these functions just helpers and use calculated world matrices, so use it only:
    // - after transform invalidation phase
    // - if node has own Transform2D components
    static void fastLocalToLocal(ecs::EntityApi src, ecs::EntityApi dst, float2 pos, float2& out);
};

EK_DECLARE_TYPE(Transform2D);

/** system to invalidate matrix and color in world space **/
//void updateWorldTransform2D(ecs::EntityApi root);
void updateWorldTransformAll(ecs::World* w, ecs::EntityApi root);

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

inline void setPosition(ecs::EntityApi e, float2 pos) {
    e.get_or_create<Transform2D>().setPosition(pos);
}

inline float2 getPosition(const ecs::EntityApi e) {
    return e.get_or_default<Transform2D>().getPosition();
}

inline void setRotation(ecs::EntityApi e, float radians) {
    e.get_or_create<Transform2D>().setRotation(radians);
}

inline float getRotation(const ecs::EntityApi e) {
    return e.get_or_default<Transform2D>().getRotation();
}

inline void setScale(ecs::EntityApi e, float2 sc) {
    e.get_or_create<Transform2D>().setScale(sc);
}

inline void setScale(ecs::EntityApi e, float xy) {
    e.get_or_create<Transform2D>().setScale(xy, xy);
}

inline float2 getScale(ecs::EntityApi e) {
    return e.get_or_default<Transform2D>().getScale();
}

}


