#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/math/Vec.hpp>
#include <ek/math/Rect.hpp>
#include <ek/math/Matrix.hpp>
#include <ek/ds/FixedArray.hpp>

namespace ek {

struct Camera2D {
    inline static constexpr unsigned MaxCount = 16;

    bool enabled = true;
    bool interactive = true;
    bool occlusionEnabled = true;

    int order = 0;
    int layerMask = 0xFF;

    ecs::EntityRef viewportNode;
    ecs::EntityRef root;
    float contentScale = 1;

    Vec2f relativeOrigin{};

    bool clearColorEnabled = false;
    Vec4f clearColor{0.5f, 0.5f, 0.5f, 1.0f};

    // just for fun (quad is used to clear.
    // todo: know about sync/de-sync clearing surface/RT?)
    Vec4f clearColor2{};

    bool debugOcclusion = false;
    bool debugVisibleBounds = false;
    bool debugGizmoHitTarget = false;
    bool debugGizmoPointer = false;
    bool debugGizmoSelf = false;
    bool debugDrawScriptGizmo = false;
    float debugDrawScale = 1;

    Matrix3x2f screenToWorldMatrix{};
    Matrix3x2f worldToScreenMatrix{};
    Rect2f screenRect = Rect2f::zero_one;
    Rect2f worldRect = Rect2f::zero_one;

    static ecs::EntityApi Main;
public:
    Camera2D() = default;

    explicit Camera2D(ecs::EntityApi root);

    [[nodiscard]] Matrix3x2f getMatrix(ecs::EntityApi view, float scale, const Vec2f& screenOffset, const Vec2f& screenSize) const;

public:
    static void updateQueue();

    static void render();

    static void drawGizmo(Camera2D& camera);

    static FixedArray<ecs::EntityRef, MaxCount>& getCameraQueue();

    static const Camera2D* getCurrentRenderingCamera();
};

ECX_TYPE(10, Camera2D);
}

