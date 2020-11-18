#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/math/vec.hpp>
#include <ek/math/box.hpp>
#include <ek/math/mat.hpp>

namespace ek {

struct Camera2D {
    bool enabled = true;
    bool interactive = true;
    bool occlusionEnabled = true;

    int order = 0;
    int layerMask = 0xFF;

    ecs::entity root;
    float contentScale = 1;

    rect_f viewport = rect_f::zero_one;
    float2 relativeOrigin{};

    // TODO: maybe remove, it's calculated and applied to matrix of entity by canvas scale system...
    bool syncContentScale = false;

    bool clearColorEnabled = false;
    float4 clearColor{0.5f, 0.5f, 0.5f, 1.0f};

    // just for fun (quad is used to clear.
    // todo: know about sync/de-sync clearing surface/RT?)
    float4 clearColor2 = float4::zero;

    bool debugOcclusion = false;
    bool debugVisibleBounds = false;
    bool debugGizmoHitTarget = false;
    bool debugGizmoPointer = false;
    bool debugGizmoSelf = false;
    bool debugDrawScriptGizmo = false;
    float debugDrawScale = 1;

    matrix_2d matrix{};
    matrix_2d inverseMatrix{};
    rect_f screenRect = rect_f::zero_one;
    rect_f worldRect = rect_f::zero_one;

    static ecs::entity Main;
public:
    Camera2D() = default;

    explicit Camera2D(ecs::entity root);

    [[nodiscard]] matrix_2d getMatrix(ecs::entity view, float scale) const;

public:
    static void updateQueue();

    static void render();

    static void drawGizmo(Camera2D& camera);

    static std::vector<ecs::entity>& getCameraQueue();

    static const Camera2D* getCurrentRenderingCamera();
};


}

