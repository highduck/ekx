#pragma once

#include "SceneWindow.hpp"
#include <ImGuizmo/ImGuizmo.h>
#include <ek/math/MatrixCamera.hpp>
#include <ek/math/MatrixTranspose.hpp>
#include <ek/math/MatrixInverse.hpp>

namespace ek {

// matrix 2d utility
Matrix3x2f matrix3Dto2D(const Matrix4f& m) {
    return Matrix3x2f{m.m00, m.m01,
                      m.m10, m.m11,
                      m.m30, m.m31};
}

Matrix4f matrix2Dto3D(const Matrix3x2f& m) {
    Matrix4f result{};
    result.m00 = m.a;
    result.m01 = m.b;
    result.m10 = m.c;
    result.m11 = m.d;
    result.m30 = m.tx;
    result.m31 = m.ty;
    return result;
}

Vec2f SceneView2D::getMouseWorldPos(Vec2f pos) const {
    return matrix.transformInverse(pos);
}

void SceneView2D::reset() {
    matrix = Matrix3x2f{};
    position = Vec2f{};
    scale = 1.0f;
    translation = Vec2f::zero;
}

void SceneView2D::manipulateView(Vec2f mouseWorldPosition, const Rect2f& viewport) {
    if (ImGui::IsMouseDragging(ImGuiPopupFlags_MouseButtonRight)) {
        const auto delta = ImGui::GetMouseDragDelta(ImGuiPopupFlags_MouseButtonRight);
        translation.x = delta.x;
        translation.y = delta.y;
    } else {
        position += translation;
        translation = Vec2f::zero;
    }

    const auto wheel = ImGui::GetIO().MouseWheel;
    if (wheel != 0.0f) {
        float newScale = fmax(scaleMin, scale + wheel);
        float deltaScale = newScale - scale;
        scale = newScale;
        position -= mouseWorldPosition * deltaScale;
    }

    matrix.set(position + translation, Vec2f{scale, scale}, Vec2f::zero);
    projectionMatrix = ortho_2d<float>(0, 0, viewport.width, viewport.height, -1000.0f, 1000.0f);
    viewMatrix3D.setTransform2D(position + translation, scale);
}

void SceneView3D::reset() {
    position = Vec3f::zero;
    translation = Vec3f::zero;
}

Vec2f SceneView3D::getMouseWorldPos(Vec2f viewportMousePosition) const {
    // TODO:
    return viewportMousePosition;
}

Vec2f SceneView::getMouseWorldPos() const {
    const auto mousePos = ImGui::GetMousePos();
    const Vec2f pos{mousePos.x - rect.position.x,
                     mousePos.y - rect.position.y};

    if(mode2D) return view2.getMouseWorldPos(pos);
    return view3.getMouseWorldPos(pos);
}

void SceneView::reset() {
    if(mode2D) view2.reset();
    else view3.reset();
}

void SceneView::manipulateView() {
    if(mode2D) view2.manipulateView(getMouseWorldPos(), rect);
    else {
        // TODO:
    }
}

void SceneWindow::onDraw() {
    drawToolbar();

    const ImVec2 displayPos = ImGui::GetCursorScreenPos();
    const ImVec2 displaySize = ImGui::GetContentRegionAvail();
    const bool mouseInView = ImGui::IsWindowHovered() &&
                             ImGui::IsMouseHoveringRect(displayPos,
                                                        {displayPos.x + displaySize.x,
                                                         displayPos.y + displaySize.y}
                             );

    const bool canManipulateView = mouseInView && !ImGuizmo::IsUsing();
    const bool canSelectObjects = mouseInView && !(ImGuizmo::IsUsing() || ImGuizmo::IsOver());

    view.rect.set(displayPos.x, displayPos.y, displaySize.x, displaySize.y);
    if (canManipulateView) {
        view.manipulateView();
    }

    // update size
    const float k = display.info.dpiScale;
    display.info.destinationViewport = Rect2f{
            k * displayPos.x, k * displayPos.y,
            k * displaySize.x, k * displaySize.y
    };
    display.info.window.x = displaySize.x;
    display.info.window.y = displaySize.y;
    display.info.size.x = k * displaySize.x;
    display.info.size.y = k * displaySize.y;
    display.update();

    // add pass to render imgui
    if (display.color && displaySize.x > 0 && displaySize.y > 0) {
        auto texId = (void*) static_cast<uintptr_t>(display.color->image.id);

        const auto info = sg_query_image_info(display.color->image);
        const float texCoordX1 = display.info.size.x / (float)info.width;
        const float texCoordY1 = display.info.size.y / (float)info.height;

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddImage(texId, {displayPos.x, displayPos.y},
                           {displayPos.x + display.info.size.x,
                            displayPos.y + display.info.size.y},
                           {0, 0},
                           {texCoordX1, texCoordY1});
    }

    ImGuizmo::SetOrthographic(true);
    ImGuizmo::BeginFrame();
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(displayPos.x, displayPos.y, displaySize.x, displaySize.y);

    if(!view.mode2D) {
        ImGuizmo::ViewManipulate(view.view3.viewMatrix.m, 100.0f, ImVec2(displayPos.x + displaySize.x - 128.0f, displayPos.y), ImVec2(100.0f, 100.0f), 0x10101010);
    }

    bool manipulationToolSelected = currentTool >= 2 && currentTool <= 6;
    if (manipulationToolSelected) {
        if(view.mode2D) {
            manipulateObject2D();
        }
        else {
            manipulateObject3D();
        }
    }

    if (root.valid() && canSelectObjects) {
        const auto wp = view.getMouseWorldPos();
        auto target = hitTest(root.get(), wp);
        hoverTarget = ecs::EntityRef{target};
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            auto& hierarchy = Locator::get<Editor>()->hierarchy;
            hierarchy.select(target);
            hierarchy.focus(target);
        }
    }
}

SceneWindow::SceneWindow() {
    name = "SceneWindow";
    title = ICON_FA_GLOBE " Scene###SceneWindow";
    fullFrame = true;
    display.simulated = true;
    display.info.size = {120, 120};
    display.info.window = {120, 120};
    display.info.dpiScale = 1.0f;

    view.view3.projectionMatrix = perspective_rh(Math::to_radians(45.0f), 4.0f / 3.0f, 10.0f, 1000.0f);
    view.view3.viewMatrix = look_at_rh(Vec3f{0.0f, 0.0f, 100.0f}, Vec3f::zero, Vec3f{0.0f, 1.0f, 0.0f});
}


void drawBox2(const Rect2f& rc, const Matrix3x2f& m, argb32_t color1, argb32_t color2,
              bool cross = true, argb32_t fillColor = 0_argb) {

    draw2d::state.setEmptyTexture();
    if (fillColor != argb32_t::zero) {
        draw2d::state.save_matrix();
        draw2d::state.matrix = m;
        draw2d::quad(rc, fillColor);
        draw2d::state.restore_matrix();
    }
    auto v1 = m.transform(rc.x, rc.y);
    auto v2 = m.transform(rc.right(), rc.y);
    auto v3 = m.transform(rc.right(), rc.bottom());
    auto v4 = m.transform(rc.x, rc.bottom());
    draw2d::line(v1, v2, color1, color2, 1, 1);
    draw2d::line(v2, v3, color1, color2, 1, 1);
    draw2d::line(v3, v4, color1, color2, 1, 1);
    draw2d::line(v4, v1, color1, color2, 1, 1);
    if (cross) {
        draw2d::line(v1, v3, color1, color2, 1, 1);
        draw2d::line(v2, v4, color1, color2, 1, 1);
    }
}

void SceneWindow::drawSceneNode(ecs::EntityApi e) {
    if(!e.get<Node>().visible()) {
        return;
    }

    auto* disp = e.tryGet<Display2D>();
    if (disp && disp->drawable) {
        auto* transform = e.tryGet<WorldTransform2D>();
        if (transform) {
            draw2d::state.matrix = transform->matrix;
            draw2d::state.color = transform->color;
        } else {
            draw2d::state.matrix = Matrix3x2f{};
            draw2d::state.color = ColorMod32{};
        }
        disp->drawable->draw();
    }
    auto it = e.get<Node>().child_first;
    while (it) {
        drawSceneNode(it);
        it = it.get<Node>().sibling_next;
    }
}

void SceneWindow::drawSceneNodeBounds(ecs::EntityApi e) {
    if(!e.get<Node>().visible()) {
        return;
    }

    auto* disp = e.tryGet<Display2D>();
    if (disp) {
        draw2d::state.matrix = Matrix3x2f{};
        draw2d::state.color = ColorMod32{};

        Matrix3x2f m = view.view2.matrix;
        auto* transform = e.tryGet<WorldTransform2D>();
        if (transform) {
            m = view.view2.matrix * transform->matrix;
        }
        Rect2f b = disp->getBounds();
        if (Locator::get<Editor>()->hierarchy.isSelectedInHierarchy(e)) {
            drawBox2(b, m, 0xFFFFFFFF_argb, 0xFF000000_argb, true, 0x77FFFFFF_argb);
        }
        if (hoverTarget.check(e)) {
            drawBox2(b, m, 0x77FFFFFF_argb, 0x77000000_argb, false);
        }
    }
    auto it = e.get<Node>().child_first;
    while (it) {
        drawSceneNodeBounds(it);
        it = it.get<Node>().sibling_next;
    }
}

void SceneWindow::onPreRender() {
    if (!display.color) {
        return;
    }
    sg_pass_action passAction{};
    passAction.colors[0].action = SG_ACTION_CLEAR;
    passAction.colors[0].value = {0.5f, 0.5f, 0.5f, 1.0f};
    passAction.depth.action = SG_ACTION_CLEAR;
    passAction.depth.value = 1.0f;
    if (display.beginGame(passAction, "Scene")) {
        drawScene();
        display.endGame();
    }
}

void SceneWindow::drawScene() {
    if (!root.valid()) {
        root = ecs::EntityRef{Locator::get<basic_application>()->root};
    }

    draw2d::begin({0, 0, display.info.size.x, display.info.size.y}, view.view2.matrix);
    if(!view.mode2D) {
        draw2d::state.mvp = view.view3.projectionMatrix * view.view3.viewMatrix;
    }
    drawSceneNode(root.get());
    draw2d::end();

    draw2d::begin({0, 0, display.info.size.x, display.info.size.y});
    if(!view.mode2D) {
        draw2d::state.mvp = view.view3.projectionMatrix * view.view3.viewMatrix;
    }
    drawSceneNodeBounds(root.get());
    draw2d::end();
}

ecs::EntityApi SceneWindow::hitTest(ecs::EntityApi e, Vec2f worldPos) {
    const auto& node = e.get<Node>();
    if(!node.visible() || !node.touchable()) {
        return nullptr;
    }
    auto it = node.child_last;
    while (it) {
        auto t = hitTest(it, worldPos);
        if (t) {
            return t;
        }
        it = it.get<Node>().sibling_prev;
    }
    auto* disp = e.tryGet<Display2D>();
    if (disp) {
        auto* wt = e.tryGet<WorldTransform2D>();
        if (wt) {
            auto lp = wt->matrix.transformInverse(worldPos);
            if (disp->getBounds().contains(lp)) {
                return e;
            }
        }
    }
    return nullptr;
}

void SceneWindow::drawToolbar() {
    if (ImGui::BeginPopupContextItem("###scene_camera_menu")) {
        if (ImGui::MenuItem("Reset")) {
            view.reset();
        }
        ImGui::EndPopup();
    }
    if (ImGui::Button(ICON_FA_CAMERA)) {
        ImGui::OpenPopup("###scene_camera_menu");
    }
    ImGui::SameLine();
    if (ImGui::ToolbarButton("2D", view.mode2D, "Toggle 2D/3D")) {
        view.mode2D = !view.mode2D;
    }
    ImGui::SameLine();
    if (ImGui::ToolbarButton(ICON_FA_MOUSE_POINTER, currentTool == 0, "Select")) {
        currentTool = 0;
    }
    ImGui::SameLine();
    if (ImGui::ToolbarButton(ICON_FA_HAND_PAPER, currentTool == 1, "Pan")) {
        currentTool = 1;
    }
    ImGui::SameLine();
    if (ImGui::ToolbarButton(ICON_FA_ARROWS_ALT, currentTool == 2, "Translate")) {
        currentTool = 2;
    }
    ImGui::SameLine();
    if (ImGui::ToolbarButton(ICON_FA_CIRCLE, currentTool == 3, "Rotate")) {
        currentTool = 3;
    }
    ImGui::SameLine();
    if (ImGui::ToolbarButton(ICON_FA_EXPAND, currentTool == 4, "Scale")) {
        currentTool = 4;
    }
    ImGui::SameLine();
    if (ImGui::ToolbarButton(ICON_FA_ADJUST, currentTool == 5, "Manipulate")) {
        currentTool = 5;
    }
    ImGui::SameLine();
    if (ImGui::ToolbarButton(ICON_FA_VECTOR_SQUARE, currentTool == 6, "Bounds")) {
        currentTool = 6;
    }

    if (currentTool != 4) {
        ImGui::SameLine();
        if (ImGui::ToolbarButton(ICON_FA_OBJECT_GROUP, localGlobal == 0, "Object Space")) {
            localGlobal = 0;
        }
        ImGui::SameLine();
        if (ImGui::ToolbarButton(ICON_FA_OBJECT_UNGROUP, localGlobal == 1, "World Space")) {
            localGlobal = 1;
        }
    }
}

void SceneWindow::manipulateObject2D() {
    auto& selection = Locator::get<Editor>()->hierarchy.selection;
    if (selection.size() > 0 && selection[0].valid()) {
        ecs::EntityApi sel = selection[0].get();
        auto worldMatrix2D = sel.get<WorldTransform2D>().matrix;
        Matrix4f worldMatrix3D = matrix2Dto3D(worldMatrix2D);
        ImGuizmo::OPERATION op = ImGuizmo::OPERATION::BOUNDS;
        if (currentTool == 2) {
            op = ImGuizmo::OPERATION::TRANSLATE_X | ImGuizmo::OPERATION::TRANSLATE_Y;
        } else if (currentTool == 3) {
            op = ImGuizmo::OPERATION::ROTATE_Z;
        } else if (currentTool == 4) {
            op = ImGuizmo::OPERATION::SCALE;
        } else if (currentTool == 5) {
            op = ImGuizmo::OPERATION::SCALE | ImGuizmo::OPERATION::ROTATE_Z | ImGuizmo::OPERATION::TRANSLATE_X |
                 ImGuizmo::OPERATION::TRANSLATE_Y;
        }
        ImGuizmo::MODE mode = (localGlobal == 0 && currentTool != 4) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
        ImGuizmo::Manipulate(view.view2.viewMatrix3D.m, view.view2.projectionMatrix.m,
                             op, mode,
                             worldMatrix3D.m,
                             nullptr,
                             nullptr,
                             nullptr,
                             nullptr);

        auto& localTransform = sel.get<Transform2D>();
        const auto parentWorldMatrix2D = sel.get<Node>().parent.get<WorldTransform2D>().matrix;
        const auto parentWorldMatrix3D = matrix2Dto3D(parentWorldMatrix2D);
        const auto inverseParentWorldMatrix3D = inverse(parentWorldMatrix3D);
        const auto newLocalMatrix3D = inverseParentWorldMatrix3D * worldMatrix3D;
        localTransform.setMatrix(matrix3Dto2D(newLocalMatrix3D));
    }
}

void SceneWindow::manipulateObject3D() {
    auto& selection = Locator::get<Editor>()->hierarchy.selection;
    if (selection.size() > 0 && selection[0].valid()) {
        ecs::EntityApi sel = selection[0].get();
        auto worldMatrix2D = sel.get<WorldTransform2D>().matrix;
        Matrix4f worldMatrix3D = matrix2Dto3D(worldMatrix2D);
        ImGuizmo::OPERATION op = ImGuizmo::OPERATION::BOUNDS;
        if (currentTool == 2) {
            op = ImGuizmo::OPERATION::TRANSLATE_X | ImGuizmo::OPERATION::TRANSLATE_Y;
        } else if (currentTool == 3) {
            op = ImGuizmo::OPERATION::ROTATE_Z;
        } else if (currentTool == 4) {
            op = ImGuizmo::OPERATION::SCALE;
        } else if (currentTool == 5) {
            op = ImGuizmo::OPERATION::SCALE | ImGuizmo::OPERATION::ROTATE_Z | ImGuizmo::OPERATION::TRANSLATE_X |
                 ImGuizmo::OPERATION::TRANSLATE_Y;
        }
        ImGuizmo::MODE mode = (localGlobal == 0 && currentTool != 4) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
        ImGuizmo::Manipulate(view.view3.viewMatrix.m, view.view3.projectionMatrix.m,
                             op, mode,
                             worldMatrix3D.m,
                             nullptr,
                             nullptr,
                             nullptr,
                             nullptr);

//        auto& localTransform = sel.get<Transform2D>();
//        const auto parentWorldMatrix2D = sel.get<Node>().parent.get<WorldTransform2D>().matrix;
//        const auto parentWorldMatrix3D = matrix2Dto3D(parentWorldMatrix2D);
//        const auto inverseParentWorldMatrix3D = inverse(parentWorldMatrix3D);
//        const auto newLocalMatrix3D = inverseParentWorldMatrix3D * worldMatrix3D;
//        localTransform.setMatrix(matrix3Dto2D(newLocalMatrix3D));
    }
}

}