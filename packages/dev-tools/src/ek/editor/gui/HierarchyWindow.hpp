#pragma once

#include "EditorWindow.hpp"
#include <IconsFontAwesome5.h>
#include <ek/ds/Hash.hpp>

// components
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

class HierarchyWindow : public EditorWindow {
public:
    HierarchyWindow() {
        name = "HierarchyWindow";
        title = ICON_FA_SITEMAP " Hierarchy###HierarchyWindow";
    }

    ~HierarchyWindow() override = default;

    Array<ecs::EntityRef> selection{};
    ImGuiTextFilter filter{};
    ecs::EntityRef root{};
    Hash<ecs::EntityRef> openList{};
    Hash<ecs::EntityRef> scrollToList{};

    void onDraw() override;

    static ecs::EntityApi getSiblingNext(ecs::EntityApi e);

    const char* getEntityIcon(ecs::EntityApi e);

    static const char* getEntityTitle(ecs::EntityApi e);

    bool isSelectedInHierarchy(ecs::EntityApi e);

    static const void* getEntityID(ecs::EntityApi e);

    static bool hasChildren(ecs::EntityApi e);

    static bool hoverIconButton(const char* str_id, const char* icon);

    void drawVisibleTouchControls(Node* node, bool parentedVisible, bool parentedTouchable);

    void drawEntityInTree(ecs::EntityApi e, bool parentedVisible, bool parentedTouchable);

    void drawEntityFiltered(ecs::EntityApi e, bool parentedVisible, bool parentedTouchable);

    void drawFilter();

    // remove any invalid refs from selection
    void validateSelection();

    void select(ecs::EntityApi e);
    void focus(ecs::EntityApi e);
};

}