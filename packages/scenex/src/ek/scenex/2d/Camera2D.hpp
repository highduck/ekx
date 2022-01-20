#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/ds/FixedArray.hpp>
#include <ek/math.h>

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

    vec2_t relativeOrigin = vec2(0, 0);

    bool clearColorEnabled = false;
    vec4_t clearColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);

    // just for fun (quad is used to clear.
    // todo: know about sync/de-sync clearing surface/RT?)
    vec4_t clearColor2 = vec4(0, 0, 0, 0);

    bool debugOcclusion = false;
    bool debugVisibleBounds = false;
    bool debugGizmoHitTarget = false;
    bool debugGizmoPointer = false;
    bool debugGizmoSelf = false;
    bool debugDrawScriptGizmo = false;
    float debugDrawScale = 1;

    mat3x2_t screenToWorldMatrix = mat3x2_identity();
    mat3x2_t worldToScreenMatrix = mat3x2_identity();
    rect_t screenRect = rect_01();
    rect_t worldRect = rect_01();

    static ecs::EntityApi Main;
public:
    Camera2D() = default;

    explicit Camera2D(ecs::EntityApi root);

    [[nodiscard]] mat3x2_t getMatrix(ecs::EntityApi view, float scale, vec2_t screenOffset, vec2_t screenSize) const;

public:
    static void updateQueue();

    static void render();

    static void drawGizmo(Camera2D& camera);

    static FixedArray<ecs::EntityRef, MaxCount>& getCameraQueue();

    static const Camera2D* getCurrentRenderingCamera();
};


}

