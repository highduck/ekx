#pragma once

#include <ecx/ecx.hpp>
#include <ek/math.h>

namespace ek {

// 32 bytes
struct WorldTransform2D {
    mat3x2_t matrix = mat3x2_identity(); // 4 * 6 = 24
    color2_t color = color2_identity(); // 2 * 4 = 8
};

// TODO: mat2x2, position,
struct Transform2D {
    // 24
    mat3x2_t matrix = mat3x2_identity();

    // 8
    color2_t color = color2_identity();

    // 16
    vec2_t cachedScale = vec2(1, 1);
    vec2_t cachedSkew = vec2(0, 0);

    inline void setX(float x) {
        matrix.tx = x;
    }

    inline void setY(float y) {
        matrix.ty = y;
    }

    inline void setMatrix(const mat3x2_t m) {
        matrix = m;
        cachedScale = mat2_get_scale(m.rot);
        cachedSkew = mat2_get_skew(m.rot);
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
        matrix.pos.x += x;
        matrix.pos.y += y;
    }

    inline void set_position(vec2_t position_) {
        matrix.pos = position_;
    }

    [[nodiscard]]
    inline vec2_t getPosition() const {
        return matrix.pos;
    }

    inline void set_position(float x, float y) {
        matrix.tx = x;
        matrix.ty = y;
    }

    inline void set_position(vec2_t position, vec2_t pivot) {
        const auto xx = -pivot.x;
        const auto yy = -pivot.y;
        matrix.tx = position.x + matrix.a * xx + matrix.c * yy;
        matrix.ty = position.y + matrix.d * yy + matrix.b * xx;
    }

    inline void set_position(vec2_t position_, vec2_t pivot_, vec2_t origin_) {
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

    void set_scale(float value) {
        cachedScale.x = value;
        cachedScale.y = value;
        updateMatrix2x2();
    }

    void set_scale(vec2_t value) {
        cachedScale = value;
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
    inline vec2_t getScale() const {
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

    void set_rotation(float value) {
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
    inline vec2_t getSkew() const {
        return cachedSkew;
    }

    inline void setSkew(vec2_t value) {
        cachedSkew = value;
        updateMatrix2x2();
    }

    inline void setTransform(vec2_t position, vec2_t scale_, vec2_t skew_, vec2_t pivot_) {
        cachedScale = scale_;
        cachedSkew = skew_;
        updateMatrix2x2();
        set_position(position, pivot_);
    }

    void setTransform(vec2_t position_, vec2_t scale_, float rotation_) {
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

    inline void setTransform(vec2_t position_, vec2_t scale_, vec2_t skew_) {
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

    void translate(vec2_t delta) {
        set_position(getPosition() + delta);
    }

    void lerpScale(vec2_t target, float t) {
        set_scale(lerp_vec2(getScale(), target, t));
    }

    void lerpPosition(vec2_t target, float t) {
        set_position(lerp_vec2(getPosition(), target, t));
    }

    void lerpRotation(float target, float t) {
        set_rotation(lerp_f32(getRotation(), target, t));
    }

    inline void updateMatrix2x2() {
        matrix.a = cosf(cachedSkew.y) * cachedScale.x;
        matrix.b = sinf(cachedSkew.y) * cachedScale.x;
        matrix.c = -sinf(cachedSkew.x) * cachedScale.y;
        matrix.d = cosf(cachedSkew.x) * cachedScale.y;
    }
};

vec2_t transform_up(entity_t it, entity_t top, vec2_t pos);

vec2_t transform_down(entity_t top, entity_t it, vec2_t pos);

vec2_t local_to_local(entity_t src, entity_t dst, vec2_t pos);

vec2_t local_to_global(entity_t local, vec2_t localPos);

vec2_t global_to_local(entity_t local, vec2_t globalPos);

// these functions just helpers and use calculated world matrices, so use it only:
// - after transform invalidation phase
// - if node has own Transform2D components
void fast_local_to_local(entity_t src, entity_t dst, vec2_t pos, vec2_t* out);

/** system to invalidate matrix and color in world space **/
void update_world_transform_2d(entity_t root);

/** utility functions **/

inline void set_alpha(entity_t e, uint8_t alpha) {
    ecs::add<Transform2D>(e).color.scale.a = alpha;
}

inline void set_alpha_f(entity_t e, float alpha) {
    ecs::add<Transform2D>(e).color.scale.a = unorm8_f32_clamped(alpha);
}

inline void set_color_additive_f(entity_t e, float additive) {
    ecs::add<Transform2D>(e).color.offset.a = unorm8_f32_clamped(additive);
}

inline void set_color(entity_t e, color_t color) {
    ecs::add<Transform2D>(e).color.scale = color;
}

inline void set_color_offset(entity_t e, color_t offset) {
    ecs::add<Transform2D>(e).color.offset = offset;
}

inline void set_x(entity_t e, float x) {
    ecs::add<Transform2D>(e).setX(x);
}

inline void set_y(entity_t e, float y) {
    ecs::add<Transform2D>(e).setY(y);
}

inline void set_position(entity_t e, vec2_t pos) {
    ecs::add<Transform2D>(e).set_position(pos);
}

inline vec2_t get_position(entity_t e) {
    return ecs::get_or_default<Transform2D>(e).getPosition();
}

inline void set_rotation(entity_t e, float radians) {
    ecs::add<Transform2D>(e).set_rotation(radians);
}

inline float get_rotation(entity_t e) {
    return ecs::get_or_default<Transform2D>(e).getRotation();
}

inline void set_scale(entity_t e, vec2_t sc) {
    ecs::add<Transform2D>(e).set_scale(sc);
}

inline void set_scale_f(entity_t e, float xy) {
    ecs::add<Transform2D>(e).set_scale(vec2(xy, xy));
}

inline vec2_t get_scale(entity_t e) {
    return ecs::get_or_default<Transform2D>(e).getScale();
}

}


