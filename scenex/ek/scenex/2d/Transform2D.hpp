#pragma once

#include <ek/math/mat3x2.hpp>
#include <ek/math/vec.hpp>
#include <ek/math/box.hpp>
#include <ek/math/packed_color.hpp>
#include <ecxx/ecxx.hpp>

namespace ek {

struct Transform2D {

    float2 position = float2::zero;
    float2 scale = float2::one;
    float2 skew = float2::zero;
    float2 origin = float2::zero;
    float2 pivot = float2::zero;

    ColorMod32 color{};
    ColorMod32 worldColor{};

    matrix_2d matrix{};
    matrix_2d worldMatrix{};

    void rotation(float value) {
        skew = {value, value};
    }

    [[nodiscard]] float rotation() const {
        return skew.x == skew.y ? skew.y : 0.0f;
    }

    void rotate(float value) {
        skew.x += value;
        skew.y += value;
    }

    void updateLocalMatrix();

    // Update local matrices for connected nodes `src` and `dst` if common LCA entity is found
    // returns LCA entity if found, or 0
    static ecs::entity updateLocalMatrixSubTree(ecs::entity src, ecs::entity dst);

    static float2 transformUp(ecs::entity it, ecs::entity top, float2 pos);

    static float2 transformDown(ecs::entity top, ecs::entity it, float2 pos);

    static float2 localToLocal(ecs::entity src, ecs::entity dst, float2 pos);

    // these functions just helpers and use calculated world matrices, so use it only:
    // - after transform invalidation phase
    // - if node has own Transform2D components
    static bool fastLocalToLocal(ecs::entity src, ecs::entity dst, float2 pos, float2& out);
};

// TODO : remove
void begin_transform(const Transform2D& transform);

// TODO : remove
void end_transform();

/** system to invalidate matrix and color in world space **/
void updateWorldTransform2D(ecs::entity root);

/** utility functions **/

inline void setAlpha(ecs::entity e, float alpha) {
    ecs::get_or_create<Transform2D>(e).color.setAlpha(alpha);
}

inline void setColorScale(ecs::entity e, argb32_t color_multiplier) {
    ecs::get_or_create<Transform2D>(e).color.scale = color_multiplier;
}

inline void setColorOffset(ecs::entity e, argb32_t color_offset) {
    ecs::get_or_create<Transform2D>(e).color.offset = color_offset;
}

inline void setPosition(ecs::entity e, const float2& pos) {
    ecs::get_or_create<Transform2D>(e).position = pos;
}

inline float2 getPosition(const ecs::entity e) {
    return ecs::get_or_default<Transform2D>(e).position;
}

inline void setRotation(ecs::entity e, float radians) {
    ecs::get_or_create<Transform2D>(e).rotation(radians);
}

inline float getRotation(const ecs::entity e) {
    return ecs::get_or_default<Transform2D>(e).rotation();
}

inline void setScale(ecs::entity e, const float2& sc) {
    ecs::get_or_create<Transform2D>(e).scale = sc;
}

inline void setScale(ecs::entity e, float xy) {
    ecs::get_or_create<Transform2D>(e).scale = {xy, xy};
}

inline float2 getScale(ecs::entity e) {
    return ecs::get_or_default<Transform2D>(e).scale;
}

}


