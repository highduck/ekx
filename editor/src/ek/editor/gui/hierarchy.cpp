#include "gui.hpp"

#include <ek/editor/imgui/imgui.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/base/Script.hpp>
#include <ek/scenex/base/Interactive.hpp>
#include <ek/scenex/3d/Light3D.hpp>
#include <ek/scenex/2d/MovieClip.hpp>
#include <ek/scenex/2d/Button.hpp>
#include <ek/scenex/app/basic_application.hpp>
#include <ek/scenex/3d/Transform3D.hpp>

namespace ek {

void gui_entity(ecs::entity e, bool visible, bool touchable);

void gui_entity_simple(ecs::entity e);

void gui_entity_node(ecs::entity e, bool visible, bool touchable);

void gui_entity_simple(ecs::entity e) {
    ImGui::LabelText("entity", "id: %i (v.%i)", e.index, ecs::the_world.generations[e.index]);
    ImGui::SameLine();
    if (ImGui::Button("-")) {
        ecs::destroy(e);
    }
}

void gui_entity(ecs::entity e, bool visible, bool touchable) {
    ImGui::PushID(e.index);
    ImGui::BeginGroup();
    if (!e.isAlive()) {
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
    if (e.has<NodeName>()) {
        const auto& nameData = e.get<NodeName>().name;
        if (!nameData.empty()) {
            return nameData.c_str();
        }
    }

    const char* type = "Entity";
    if (e.has<Node>()) type = "Node";
    if (e.has<Transform2D>()) type = "Node 2D";
    if (e.has<Transform3D>()) type = "Node 3D";
    if (e.has<Display2D>()) {
        const auto& disp = e.get<Display2D>();
        if (disp.is<Sprite2D>()) type = "Sprite";
        if (disp.is<Quad2D>()) type = "Rectangle";
        if (disp.is<Text2D>()) type = "Text";
        if (disp.is<Arc2D>()) type = "Arc";
        // TODO: other renderables
    }
    if (e.has<MovieClip>()) { type = "MovieClip"; }
    if (e.has<Interactive>()) { type = "Interactive"; }
    if (e.has<Button>()) { type = "Button"; }
    if (e.has<Bounds2D>()) {
        type = "Bounds2D";
    }
    if (e.has<ScriptHolder>()) { type = "Script"; }
    return type;
}

bool isSelectedInHierarchy(ecs::entity e) {
    auto it = std::find(hierarchySelectionList.begin(), hierarchySelectionList.end(), e);
    return it != hierarchySelectionList.end();
}

bool hasChildren(ecs::entity e) {
    if (e.has<Node>()) {
        auto& node = e.get<Node>();
        auto firstChild = node.child_first;
        return firstChild != nullptr && firstChild.isAlive();
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

    if (e.has<Node>()) {
        const auto& node = e.get<Node>();
        visible = visible && node.visible();
        touchable = touchable && node.touchable();
    }
    ImGui::PushStyleColor(ImGuiCol_Text, visible ? 0xFFFFFFFF : 0x77FFFFFF);
    bool opened = ImGui::TreeNodeEx("hierarchy_node", flags, "%s", getEntityTitle(e));

    if (ImGui::IsItemClicked()) {
        hierarchySelectionList.clear();
        hierarchySelectionList.push_back(e);
    }

    if (e.has<Node>()) {
        auto& node = e.get<Node>();
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
        if (e.has<Node>()) {
            auto it = e.get<Node>().child_first;
            while (it) {
                auto child = it;
                it = it.get<Node>().sibling_next;
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
            ecs::each([](ecs::entity e) {
                auto* node = e.tryGet<Node>();
                if (!node || !node->parent) {
                    gui_entity(e, true, true);
                }
            });
        } else {
            // ROOT
            auto& app = resolve<basic_application>();
            auto root = app.root;
            if (root.isAlive()) {
                auto it = root.get<Node>().child_first;
                while (it) {
                    gui_entity(it, true, true);
                    it = it.get<Node>().sibling_next;
                }
            } else {
                ImGui::TextColored({1, 0, 0, 1}, "Invalid ROOT container");
            }
        }
    }
    ImGui::End();
}

}