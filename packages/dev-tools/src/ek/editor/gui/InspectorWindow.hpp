#pragma once

#include "EditorWindow.hpp"

namespace ek {

class InspectorWindow : public EditorWindow {
public:
    PodArray<ecs::Entity> list{};

    InspectorWindow() {
        name = "InspectorWindow";
        title = ICON_FA_PUZZLE_PIECE " Inspector###InspectorWindow";
    }

    ~InspectorWindow() override = default;

    void gui_inspector(ecs::Entity entity);

    void onDraw() override;
};

}