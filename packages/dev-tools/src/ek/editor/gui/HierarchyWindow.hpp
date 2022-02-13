#pragma once

#include "EditorWindow.hpp"
#include <IconsFontAwesome5.h>
#include <ek/ds/Hash.hpp>

// components
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

class HierarchyWindow : public EditorWindow {
public:
    HierarchyWindow() {
        name = "HierarchyWindow";
        title = ICON_FA_SITEMAP " Hierarchy###HierarchyWindow";
    }

    ~HierarchyWindow() override = default;

    PodArray<ecs::Entity> selection{};
    ImGuiTextFilter filter{};
    ecs::Entity root{};
    Hash<ecs::Entity> openList{};
    Hash<ecs::Entity> scrollToList{};

    void onDraw() override;

    static entity_t getSiblingNext(entity_t e);

    const char* getEntityIcon(entity_t e);

    bool isSelectedInHierarchy(entity_t e);

    static bool hasChildren(entity_t e);

    static bool hoverIconButton(const char* str_id, const char* icon);

    void drawVisibleTouchControls(Node* node, bool parentedVisible, bool parentedTouchable);

    void drawEntityInTree(entity_t e, bool parentedVisible, bool parentedTouchable);

    void drawEntityFiltered(entity_t e, bool parentedVisible, bool parentedTouchable);

    void drawFilter();

    // remove any invalid refs from selection
    void validateSelection();

    void select(entity_t e);
    void focus(entity_t e);
};

}