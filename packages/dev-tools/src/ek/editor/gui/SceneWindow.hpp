#pragma once

#include "EditorWindow.hpp"
#include <ek/scenex/app/GameDisplay.hpp>

namespace ek {

struct SceneView2D {
    float scaleMin = 0.1f;
    float scale = 1.0f;
    Vec2f position{};
    Matrix3x2f matrix{};
    Vec2f translation{};
    Matrix4f viewMatrix3D{};
    Matrix4f projectionMatrix{};

    [[nodiscard]]
    Vec2f getMouseWorldPos(Vec2f viewportMousePosition) const;
    void reset();
    void manipulateView(Vec2f mouseWorldPosition, const Rect2f& viewport);
};

struct SceneView3D {
    Vec3f position{};
    Vec3f translation{};
    Matrix4f viewMatrix{};
    Matrix4f projectionMatrix{};

    [[nodiscard]]
    Vec2f getMouseWorldPos(Vec2f viewportMousePosition) const;
    void reset();
//    void manipulateView();
};

struct SceneView {
    SceneView2D view2;
    SceneView3D view3;
    bool mode2D = true;
    Rect2f rect{0, 0, 1, 1};

    [[nodiscard]]
    Vec2f getMouseWorldPos() const;
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

    ecs::EntityApi hitTest(ecs::EntityApi e, Vec2f worldPos);

    void drawToolbar();

    void manipulateObject2D();
    void manipulateObject3D();
};

}