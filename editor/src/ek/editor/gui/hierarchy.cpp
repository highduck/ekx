#include "gui.hpp"

#include <ek/editor/imgui/imgui.hpp>
#include <ek/scenex/components/node.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/components/script.hpp>
#include <ek/scenex/components/interactive.hpp>
#include <ek/scenex/components/event_handler.hpp>
#include <ek/scenex/3d/Transform3D.hpp>
#include <ek/scenex/3d/Camera3D.hpp>
#include <ek/math/common.hpp>
#include <ek/scenex/3d/Light3D.hpp>
#include <ek/scenex/components/movie.hpp>
#include <ek/scenex/components/button.hpp>
#include <ek/scenex/app/basic_application.hpp>

namespace ek {

void gui_entity(ecs::entity e, bool visible, bool touchable);

void gui_entity_simple(ecs::entity e);

void gui_entity_node(ecs::entity e, bool visible, bool touchable);

void gui_entity_simple(ecs::entity e) {
    ImGui::LabelText("entity", "id: %i (v.%i)", e.index(), e.version());
    ImGui::SameLine();
    if (ImGui::Button("-")) {
        ecs::destroy(e);
    }
}

void gui_entity(ecs::entity e, bool visible, bool touchable) {
    ImGui::PushID(e.passport());
    ImGui::BeginGroup();
    if (!ecs::valid(e)) {
        ImGui::Text("INVALID ENTITY");
    } else {
        gui_entity_node(e, visible, touchable);
    }
    ImGui::EndGroup();
    ImGui::PopID();
}

std::vector<ecs::entity> hierarchySelectionList{};
std::string hierarchyFilterText{};

const char* getEntityTitle(ecs::entity e) {
    if (ecs::has<Node>(e)) {
        const auto& nameData = ecs::get<Node>(e).name;
        if (!nameData.empty()) {
            return nameData.c_str();
        }
    }

    const char* type = "Entity";
    if (ecs::has<Node>(e)) type = "Node";
    if (ecs::has<Transform2D>(e)) type = "Node 2D";
    if (ecs::has<Transform3D>(e)) type = "Node 3D";
    if (ecs::has<Display2D>(e)) {
        const auto& disp = ecs::get<Display2D>(e);
        if (disp.is<Sprite2D>()) type = "Sprite";
        if (disp.is<Quad2D>()) type = "Rectangle";
        if (disp.is<Text2D>()) type = "Text";
        if (disp.is<Arc2D>()) type = "Arc";
    }
    if (ecs::has<movie_t>(e)) { type = "MovieClip"; }
    if (ecs::has<interactive_t>(e)) { type = "Interactive"; }
    if (ecs::has<button_t>(e)) { type = "Button"; }
    if (ecs::has<scissors_2d>(e)) { type = "Scissors"; }
    if (ecs::has<script_holder>(e)) { type = "Script"; }
    return type;
}

bool isSelectedInHierarchy(ecs::entity e) {
    auto it = std::find(hierarchySelectionList.begin(), hierarchySelectionList.end(), e);
    return it != hierarchySelectionList.end();
}

bool hasChildren(ecs::entity e) {
    if (ecs::has<Node>(e)) {
        auto& node = ecs::get<Node>(e);
        auto firstChild = node.child_first;
        return firstChild != nullptr && ecs::valid(firstChild);
    }
    return false;
}

void gui_entity_node(ecs::entity e, bool visible, bool touchable) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
                               | ImGuiTreeNodeFlags_OpenOnDoubleClick
                               | ImGuiTreeNodeFlags_AllowItemOverlap
                               | ImGuiTreeNodeFlags_SpanFullWidth;
    if (isSelectedInHierarchy(e)) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (!hasChildren(e)) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (ecs::has<Node>(e)) {
        const auto& node = ecs::get<Node>(e);
        visible = visible && node.visible();
        touchable = touchable && node.touchable();
    }
    ImGui::PushStyleColor(ImGuiCol_Text, visible ? 0xFFFFFFFF : 0x77FFFFFF);
    bool opened = ImGui::TreeNodeEx("hierarchy_node", flags, "%s", getEntityTitle(e));

    if (ImGui::IsItemClicked()) {
        hierarchySelectionList.clear();
        hierarchySelectionList.push_back(e);
    }

    if (ecs::has<Node>(e)) {
        auto& node = ecs::get<Node>(e);
        auto rc = ImGui::GetItemRectMax();
        auto wnd = ImGui::GetWindowPos();
        auto x = rc.x - wnd.x - 40 - ImGui::GetStyle().FramePadding.x * 2.0f;
        ImGui::SameLine(0, 0);
        ImGui::SetCursorPosX(x);
        if (ImGui::SmallButton(node.visible() ? "O" : "-")) {
            node.setVisible(!node.visible());
        }
        ImGui::SameLine(0, 0);
        ImGui::SetCursorPosX(x + 20);
        if (ImGui::SmallButton(node.touchable() ? "T" : ".")) {
            node.setTouchable(!node.touchable());
        }
    }
    ImGui::PopStyleColor();

    if (opened) {
        if (ecs::has<Node>(e)) {
            auto it = ecs::get<Node>(e).child_first;
            while (it != nullptr && ecs::valid(it)) {
                auto child = it;
                it = ecs::get<Node>(it).sibling_next;
                gui_entity(child, visible, touchable);
            }
        }
        ImGui::TreePop();
    }
}

void guiHierarchyWindow(bool* p_open) {
    if (ImGui::Begin("Hierarchy", p_open)) {
        ImGui::InputText("Filter", &hierarchyFilterText);
        ImGui::SameLine();
        if (ImGui::Button("X")) {
            hierarchyFilterText.clear();
        }

        static bool showAllRoots = false;
        ImGui::Checkbox("All Roots", &showAllRoots);

        if (showAllRoots) {
            // all roots
            ecs::each([](auto e) {
                if (!ecs::has<Node>(e) || ecs::get<Node>(e).parent == nullptr) {
                    gui_entity(e, true, true);
                }
            });
        } else {
            // game container
            auto& app = resolve<basic_application>();
            if (ecs::valid(app.game)) {
                gui_entity(app.game, true, true);
            } else {
                ImGui::TextColored({1, 0, 0, 1}, "Invalid Game container");
            }
        }
    }
    ImGui::End();
}


}