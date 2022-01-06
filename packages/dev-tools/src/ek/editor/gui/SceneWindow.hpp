#pragma once

#include "EditorWindow.hpp"
#include <ek/scenex/app/GameDisplay.hpp>

namespace ek {

struct SceneView2D {
    float scaleMin = 0.1f;
    float scale = 1.0f;
    vec2_t position = {};
    mat3x2_t matrix = mat3x2_identity();
    vec2_t translation = {};
    mat4_t viewMatrix3D = mat4_identity();
    mat4_t projectionMatrix= mat4_identity();

    [[nodiscard]]
    vec2_t getMouseWorldPos(vec2_t viewportMousePosition) const;
    void reset();
    void manipulateView(vec2_t mouseWorldPosition, rect_t viewport);
};

struct SceneView3D {
    vec3_t position = {};
    vec3_t translation = {};
    mat4_t viewMatrix = mat4_identity();
    mat4_t projectionMatrix= mat4_identity();

    [[nodiscard]]
    vec2_t getMouseWorldPos(vec2_t viewportMousePosition) const;
    void reset();
//    void manipulateView();
};

struct SceneView {
    SceneView2D view2;
    SceneView3D view3;
    bool mode2D = true;
    rect_t rect = rect_01();

    [[nodiscard]]
    vec2_t getMouseWorldPos() const;
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

    ecs::EntityApi hitTest(ecs::EntityApi e, vec2_t worldPos);

    void drawToolbar();

    void manipulateObject2D();
    void manipulateObject3D();
};

}