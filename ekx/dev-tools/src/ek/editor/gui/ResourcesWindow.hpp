#pragma once

#include "EditorWindow.hpp"

namespace ek {

class ResourcesWindow : public EditorWindow {
public:

    ResourcesWindow() {
        name = "ResourcesWindow";
        title = ICON_FA_HDD " Resources###ResourcesWindow";
    }

    ~ResourcesWindow() override = default;

    void onDraw() override;
};

}