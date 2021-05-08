#pragma once


#include "HierarchyWindow.hpp"
#include <ek/editor/imgui/imgui.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/base/Script.hpp>
#include <ek/scenex/base/Interactive.hpp>
#include <ek/scenex/3d/Light3D.hpp>
#include <ek/scenex/2d/MovieClip.hpp>
#include <ek/scenex/2d/Button.hpp>
#include <ek/scenex/3d/Transform3D.hpp>
#include <ek/scenex/2d/Viewport.hpp>
#include <ek/scenex/2d/Camera2D.hpp>

namespace ek {

ecs::EntityApi HierarchyWindow::getSiblingNext(ecs::EntityApi e) {
    const auto* node = e.tryGet<Node>();
    return node ? node->sibling_next : nullptr;
}

const char* HierarchyWindow::getEntityIcon(ecs::EntityApi e) {
    if (e.has<Camera2D>()) return ICON_FA_VIDEO;
    if (e.has<Viewport>()) return ICON_FA_TV;
    if (e.has<ScriptHolder>()) return ICON_FA_CODE;
    if (e.has<Bounds2D>()) return ICON_FA_EXPAND;
    if (e.has<Button>()) return ICON_FA_HAND_POINTER;
    if (e.has<Interactive>()) return ICON_FA_FINGERPRINT;
    if (e.has<MovieClip>()) return ICON_FA_FILM;

    if (e.has<Display2D>()) {
        const auto& disp = e.get<Display2D>();
        if (disp.is<Sprite2D>()) return ICON_FA_IMAGE;
        if (disp.is<NinePatch2D>()) return ICON_FA_COMPRESS;
        if (disp.is<Quad2D>()) return ICON_FA_VECTOR_SQUARE;
        if (disp.is<Text2D>()) return ICON_FA_FONT;
        if (disp.is<Arc2D>()) return ICON_FA_CIRCLE_NOTCH;
        return ICON_FA_PAINT_BRUSH;
    }

    if (e.has<Transform3D>()) return ICON_FA_DICE_D20;
    if (e.has<Transform2D>()) return ICON_FA_DICE_D6;
    if (e.has<Node>()) return ICON_FA_BOX;

    return ICON_FA_BORDER_STYLE;
}

const char* HierarchyWindow::getEntityTitle(ecs::EntityApi e) {
    if (e.has<NodeName>()) {
        return e.get<NodeName>().name.c_str();
    }
    return "Entity";
}

bool HierarchyWindow::isSelectedInHierarchy(ecs::EntityApi e) {
    const ecs::EntityRef ref{e};
    auto it = std::find(selection.begin(), selection.end(), ref);
    return it != selection.end();
}

const void* HierarchyWindow::getEntityID(ecs::EntityApi e) {
    return reinterpret_cast<const void*>(ecs::EntityRef{e}.passport);
}

bool HierarchyWindow::hasChildren(ecs::EntityApi e) {
    if (e.has<Node>()) {
        auto& node = e.get<Node>();
        auto first = node.child_first;
        return first != nullptr && first.isAlive();
    }
    return false;
}

bool HierarchyWindow::hoverIconButton(const char* str_id, const char* icon) {
    ImGui::TextUnformatted(icon);
    return ImGui::IsItemClicked();
}

void HierarchyWindow::drawVisibleTouchControls(Node* node, bool parentedVisible, bool parentedTouchable) {
    if (!node) {
        return;
    }
    ImGui::SameLine(0, 0);
    ImGui::SetCursorPosX(10);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, (parentedVisible || !node->visible()) ? 1.0f : 0.5f);
    {
        const char* icon = node->visible() ? ICON_FA_EYE : ICON_FA_EYE_SLASH;
        if (hoverIconButton("visible", icon)) {
            node->setVisible(!node->visible());
        }
    }
    ImGui::PopStyleVar();

    ImGui::SameLine(0, 0);
    ImGui::SetCursorPosX(30);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, (parentedTouchable || !node->touchable()) ? 1.0f : 0.5f);
    {
        const char* icon = node->touchable() ? ICON_FA_HAND_POINTER : ICON_FA_STOP;
        if (hoverIconButton("touchable", icon)) {
            node->setTouchable(!node->touchable());
        }
    }
    ImGui::PopStyleVar();
}

void HierarchyWindow::drawEntityInTree(ecs::EntityApi e, bool parentedVisible, bool parentedTouchable) {
    if (!e.isAlive()) {
        ImGui::Text("INVALID ENTITY");
        return;
    }

    ImGui::PushID(getEntityID(e));
//        ImGui::BeginGroup();

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
                               | ImGuiTreeNodeFlags_OpenOnDoubleClick
                               | ImGuiTreeNodeFlags_AllowItemOverlap
                               | ImGuiTreeNodeFlags_SpanFullWidth
                               | ImGuiTreeNodeFlags_NavLeftJumpsBackHere;
    if (isSelectedInHierarchy(e)) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (!hasChildren(e)) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    auto nodeVisible = parentedVisible;
    auto nodeTouchable = parentedTouchable;
    auto* node = e.tryGet<Node>();
    if (node) {
        nodeVisible = nodeVisible && node->visible();
        nodeTouchable = nodeTouchable && node->touchable();
    }

    ImGui::PushStyleColor(ImGuiCol_Text, nodeVisible ? 0xFFFFFFFF : 0x77FFFFFF);
    bool opened = ImGui::TreeNodeEx("entity", flags, "%s %s", getEntityIcon(e), getEntityTitle(e));
    ImGui::PopStyleColor();

    if (ImGui::IsItemClicked()) {
        selection.clear();
        selection.push_back(ecs::EntityRef{e});
    }

    drawVisibleTouchControls(node, parentedVisible, parentedTouchable);

    if (opened) {
        if (node) {
            auto it = node->child_first;
            while (it) {
                drawEntityInTree(it, nodeVisible, nodeTouchable);
                it = getSiblingNext(it);
            }
        }
        ImGui::TreePop();
    }

//        ImGui::EndGroup();
    ImGui::PopID();
}

void HierarchyWindow::drawEntityFiltered(ecs::EntityApi e, bool parentedVisible, bool parentedTouchable) {
    if (!e.isAlive()) {
        ImGui::Text("INVALID ENTITY");
        return;
    }
    auto* node = e.tryGet<Node>();
    auto* name = e.tryGet<NodeName>();
    auto nodeVisible = parentedVisible;
    auto nodeTouchable = parentedTouchable;
    if (node) {
        nodeVisible = nodeVisible && node->visible();
        nodeTouchable = nodeTouchable && node->touchable();
    }

    if (name && filter.PassFilter(name->name.c_str())) {
        ImGui::PushID(static_cast<int>(ecs::EntityRef{e}.passport));
        ImGui::BeginGroup();

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
                                   | ImGuiTreeNodeFlags_OpenOnDoubleClick
                                   | ImGuiTreeNodeFlags_AllowItemOverlap
                                   | ImGuiTreeNodeFlags_SpanFullWidth
                                   | ImGuiTreeNodeFlags_NavLeftJumpsBackHere;
        if (isSelectedInHierarchy(e)) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        // display all filtered entities just like a list
        flags |= ImGuiTreeNodeFlags_Leaf;

        ImGui::PushStyleColor(ImGuiCol_Text, nodeVisible ? 0xFFFFFFFF : 0x77FFFFFF);
        const bool opened = ImGui::TreeNodeEx("hierarchy_node", flags, "%s%s", getEntityIcon(e), getEntityTitle(e));
        ImGui::PopStyleColor();

        if (ImGui::IsItemClicked()) {
            selection.clear();
            selection.push_back(ecs::EntityRef{e});
        }

        drawVisibleTouchControls(node, parentedVisible, parentedTouchable);

        if (opened) {
            ImGui::TreePop();
        }

        ImGui::EndGroup();
        ImGui::PopID();
    }

    if (node) {
        auto it = node->child_first;
        while (it) {
            drawEntityFiltered(it, nodeVisible, nodeTouchable);
            it = getSiblingNext(it);
        }
    }
}

void HierarchyWindow::drawFilter() {
    filter.Draw(ICON_FA_SEARCH "##hierarchy_filter", 100.0f);
    if (filter.IsActive()) {
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_TIMES_CIRCLE)) {
            filter.Clear();
        }
    }
}

void HierarchyWindow::onDraw() {
    drawFilter();
    if (!root.valid()) {
        ImGui::TextColored({1, 0, 0, 1}, "No roots");
    } else {
        ImGui::Indent(40.0f);
        if (filter.IsActive()) {
            drawEntityFiltered(root.get(), true, true);
        } else {
            drawEntityInTree(root.get(), true, true);
        }
        ImGui::Unindent(40.0f);
    }
}

// remove any invalid refs from selection
void HierarchyWindow::validateSelection() {
    unsigned i = 0;
    while (i < selection.size()) {
        if (selection[i].valid()) {
            ++i;
        } else {
            selection.eraseAt(i);
        }
    }
}

}