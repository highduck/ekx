#pragma once

#include "EditorWindow.hpp"
#include <ek/scenex/app/GameDisplay.hpp>

namespace ek {

struct SceneView2D {
    float scaleMin = 0.1f;
    float scale = 1.0f;
    float2 position{};
    matrix_2d matrix{};
    float2 translation{};
    mat4f viewMatrix3D{};
    mat4f projectionMatrix{};

    [[nodiscard]]
    float2 getMouseWorldPos(float2 viewportMousePosition) const;
    void reset();
    void manipulateView(float2 mouseWorldPosition, const rect_f& viewport);
};

struct SceneView3D {
    float3 position{};
    float3 translation{};
    mat4f viewMatrix{};
    mat4f projectionMatrix{};

    [[nodiscard]]
    float2 getMouseWorldPos(float2 viewportMousePosition) const;
    void reset();
//    void manipulateView();
};

struct SceneView {
    SceneView2D view2;
    SceneView3D view3;
    bool mode2D = true;
    rect_f rect{0, 0, 1, 1};

    [[nodiscard]]
    float2 getMouseWorldPos() const;
    void reset();
    void manipulateView();
};


class SceneWindow : public EditorWindow {
public:

    SceneWindow();

    ~SceneWindow() override = default;

    void onDraw() override;

    GameDisplay display;
    SceneView view{};

    ecs::EntityRef root{};
    ecs::EntityRef hoverTarget{};

    int currentTool = 0;
    int localGlobal = 0;

    void onPreRender();

    void drawScene();

    void drawSceneNode(ecs::EntityApi e);
    void drawSceneNodeBounds(ecs::EntityApi e);

    ecs::EntityApi hitTest(ecs::EntityApi e, float2 worldPos);

    void drawToolbar();

    void manipulateObject2D();
    void manipulateObject3D();
};

}