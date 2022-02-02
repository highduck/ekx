#pragma once

#include "EditorWindow.hpp"

namespace ek {

class InspectorWindow : public EditorWindow {
public:
    PodArray<ecs::EntityRef> list{};

    InspectorWindow() {
        name = "InspectorWindow";
        title = ICON_FA_PUZZLE_PIECE " Inspector###InspectorWindow";
    }

    ~InspectorWindow() override = default;

    void gui_inspector(ecs::EntityRef entity);

    void onDraw() override;
};

}