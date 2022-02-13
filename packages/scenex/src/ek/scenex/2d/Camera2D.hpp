#pragma once

#include <ecx/ecx.hpp>
#include <ek/ds/FixedArray.hpp>
#include <ek/math.h>

namespace ek {

struct Camera2D {
    inline static constexpr unsigned MaxCount = 16;

    int order = 0;

    entity_t viewportNode = NULL_ENTITY;
    entity_t root = NULL_ENTITY;
    float contentScale = 1;

    vec2_t relativeOrigin = vec2(0, 0);

    bool clearColorEnabled = false;
    vec4_t clearColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);

    // just for fun (quad is used to clear.
    // todo: know about sync/de-sync clearing surface/RT?)
    vec4_t clearColor2 = vec4(0, 0, 0, 0);

    bool enabled = true;
    bool interactive = true;
    bool occlusionEnabled = true;
    bool debugOcclusion = false;
    bool debugVisibleBounds = false;
    bool debugGizmoHitTarget = false;
    bool debugGizmoPointer = false;
    bool debugGizmoSelf = false;
    float debugDrawScale = 1;

    mat3x2_t screenToWorldMatrix = mat3x2_identity();
    mat3x2_t worldToScreenMatrix = mat3x2_identity();
    rect_t screenRect = rect_01();
    rect_t worldRect = rect_01();

    static entity_t Main;
public:
    Camera2D() = default;

    [[nodiscard]] mat3x2_t getMatrix(entity_t view, float scale, vec2_t screenOffset, vec2_t screenSize) const;

public:
    static void drawGizmo(Camera2D& camera);

    static FixedArray<entity_t, MaxCount>& getCameraQueue();

    static const Camera2D* getCurrentRenderingCamera();
};

void update_camera2d_queue();

void render_camera2d_queue();


}

