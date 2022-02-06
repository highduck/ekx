#pragma once


#include "HierarchyWindow.hpp"
#include <ek/editor/imgui/imgui.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/2d/Transform2D.hpp>
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

    if (e.has<Bounds2D>()) return ICON_FA_EXPAND;
    if (e.has<Button>()) return ICON_FA_HAND_POINTER;
    if (e.has<Interactive>()) return ICON_FA_FINGERPRINT;
    if (e.has<MovieClip>()) return ICON_FA_FILM;

    if (e.has<Sprite2D>()) return ICON_FA_IMAGE;
    if (e.has<NinePatch2D>()) return ICON_FA_COMPRESS;
    if (e.has<Quad2D>()) return ICON_FA_VECTOR_SQUARE;
    if (e.has<Text2D>()) return ICON_FA_FONT;
    if (e.has<Arc2D>()) return ICON_FA_CIRCLE_NOTCH;
    // other displays
    if (e.has<Display2D>()) return ICON_FA_PAINT_BRUSH;

    if (ecs::hasComponent<Transform3D>() && e.has<Transform3D>()) return ICON_FA_DICE_D20;
    if (e.has<Transform2D>()) return ICON_FA_DICE_D6;
    if (e.has<Node>()) return ICON_FA_BOX;

    return ICON_FA_BORDER_STYLE;
}

void getEntityTitle(ecs::EntityApi e, char buffer[64]) {
    auto tag = e.get<Node>().tag;
    if (tag) {
        const char* str = hsp_get(tag);
        if (*str) {
            strcpy(buffer, str);
        } else {
            ek_snprintf(buffer, 64, "[0x%08X]", tag);
        }
    } else {
        strcpy(buffer, "Entity");
    }
}

bool HierarchyWindow::isSelectedInHierarchy(ecs::EntityApi e) {
    const ecs::EntityRef ref{e};
    auto it = std::find(selection.begin(), selection.end(), ref);
    return it != selection.end();
}

const void* HierarchyWindow::getEntityID(ecs::EntityApi e) {
    return reinterpret_cast<const void*>(get_entity_passport(e.index).value);
}

bool HierarchyWindow::hasChildren(ecs::EntityApi e) {
    if (e.has<Node>()) {
        auto& node = e.get<Node>();
        auto first = node.child_first;
        return first != nullptr && first.is_alive();
    }
    return false;
}

bool HierarchyWindow::hoverIconButton(const char* str_id, const char* icon) {
    (void)str_id;
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
    if (!e.is_alive()) {
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
    if (openList.has(e.index)) {
        ImGui::SetNextItemOpen(true);
    }
    if (scrollToList.has(e.index)) {
        ImGui::SetScrollHereY();
        scrollToList.remove(e.index);
    }
    char buffer[64];
    getEntityTitle(e, buffer);
    bool opened = ImGui::TreeNodeEx("entity", flags, "%s %s", getEntityIcon(e), buffer);
    if (!opened) {
        openList.remove(e.index);
    }

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
    if (!e.is_alive()) {
        ImGui::Text("INVALID ENTITY");
        return;
    }
    auto* node = e.tryGet<Node>();
    const char* name = node ? hsp_get(node->tag) : 0;
    auto nodeVisible = parentedVisible;
    auto nodeTouchable = parentedTouchable;
    if (node) {
        nodeVisible = nodeVisible && node->visible();
        nodeTouchable = nodeTouchable && node->touchable();
    }

    if (name && *name && filter.PassFilter(name)) {
        ImGui::PushID(static_cast<int>(get_entity_passport(e.index).value));
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
        char buffer[64];
        getEntityTitle(e, buffer);
        const bool opened = ImGui::TreeNodeEx("hierarchy_node", flags, "%s%s", getEntityIcon(e), buffer);
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
//        for(auto e3d: ecs::view<Transform3D>()) {
//            if(e3d.get<NodeName>().name == "scene 3d") {
//                drawEntityInTree(e3d, true, true);
//            }
//        }
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
            selection.erase_at(i);
        }
    }
}

void HierarchyWindow::select(ecs::EntityApi e) {
    selection.clear();
    if (e) {
        selection.push_back(ecs::EntityRef{e});
    }
}

void HierarchyWindow::focus(ecs::EntityApi e) {
    if (e) {
        // open parents in hierarchy
        auto parent = e.get<Node>().parent;
        while (parent) {
            openList.set(parent.index, ecs::EntityRef{parent});
            parent = parent.get<Node>().parent;
        }

        scrollToList.set(e.index, ecs::EntityRef{e});
    }
}

}