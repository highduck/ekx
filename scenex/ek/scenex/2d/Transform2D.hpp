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

    inline void set_alpha(float alpha) {
        color.scale.af(alpha);
    }

    [[nodiscard]] inline float get_alpha() const {
        return color.scale.af();
    }

    inline void set_additive(float alpha) {
        color.offset.af(alpha);
    }

    [[nodiscard]] inline float get_additive() const {
        return color.offset.af();
    }

    void updateLocalMatrix();

    // these function use local matrix, need to rebuild local matrices for [src -> lca -> dst]
    static void updateLocalMatrixSubTree(ecs::entity src, ecs::entity dst);

    static float2 transformUp(ecs::entity it, ecs::entity top, float2 pos);

    static float2 transformDown(ecs::entity top, ecs::entity it, float2 pos);

    static float2 localToLocal(ecs::entity src, ecs::entity dst, float2 pos);

    // these functions just helpers and use calculated world matrices, so use it only:
    // - after transform invalidation phase
    // - if node has own Transform2D components
    static bool fastLocalToLocal(ecs::entity src, ecs::entity dst, float2 pos, float2& out);
};

void begin_transform(const Transform2D& transform);

void end_transform();

}


